#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

#include <Framegrabber.h>
#include <VisionaryControl.h>
#include <VisionaryTMiniData.h>

#include <GL/glew.h>

#include <opencv2/core/utils/logger.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <gui/frame.h>
#include <gui/filter_pipeline.h>
#include <gui/filter_worker.h>

#include <json.hpp>

#include <imnodes.h>
#include <gui/filter_editor.h>

#include <spdlog/spdlog.h>

std::unique_ptr<visionary::FrameGrabber<visionary::VisionaryTMiniData>> frame_grabber;
std::shared_ptr<visionary::VisionaryTMiniData> frame_data;
std::unique_ptr<visionary::VisionaryControl> visionary_control;
GLuint g_texture;

const bool open_camera(const std::string& ip, const uint16_t& port, const uint64_t& timeout_ms);

namespace ImGui
{
    void FrameWindow(const frame::Frame& frame, const char* name, bool *p_open = (bool *)0, ImGuiWindowFlags flags = 0);
}

void glfw_error_callback(int error, const char* description);

int main(int, char**)
{
#ifndef _DEBUG
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
#endif // _DEBUG

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Sick GUI", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imnodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    fprintf(stdout, "Using GLEW version %s\n", glewGetString(GLEW_VERSION));
    fprintf(stdout, "Using GL version %s\n", glGetString(GL_VERSION));
        
    open_camera("192.168.1.67", 2114, 1000);

    filter_pipeline pipeline;
    editor::filter_editor editor;
    filter::filter_worker worker;
    glGenTextures(1, &g_texture);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        editor.show();

        bool pipeline_ok = editor.create_pipeline(pipeline);
        if (pipeline_ok)
        {
            if (frame_grabber->getCurrentFrame(frame_data))
            {
                frame::Frame depth(
                    frame_data->getDistanceMap(),
                    frame_data->getHeight(),
                    frame_data->getWidth(),
                    frame_data->getFrameNum(),
                    frame_data->getTimestampMS()
                );

                auto depth_mat = frame::to_mat(depth);
                worker.put_new(depth_mat);
            }
            {
                std::cout << "current frame failed\n";
            }
        }
        else
        {
            std::cout << "pipe failed\n";
        }

        cv::Mat filtered_mat = worker.latest_mat();
        frame::Frame filtered_frame;
        if (!filtered_mat.empty())
            filtered_frame = frame::to_frame(filtered_mat);
        else
            std::cerr << "EMPTY MAT!\n";
        ImGui::FrameWindow(filtered_frame, "Filtered Frame");

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.5, 0.5, 0.5, 1.0);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteTextures(1, &g_texture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    imnodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

const bool open_camera(const std::string& ip, const uint16_t& port, const uint64_t& timeout_ms)
{ 
    frame_grabber.reset(new visionary::FrameGrabber<visionary::VisionaryTMiniData>(ip, htons(port), timeout_ms));
    if (!frame_grabber)
    {
        spdlog::error("Failed to create frame grabber");
        return false;
    }

    frame_data.reset(new visionary::VisionaryTMiniData);
    if (!frame_data)
    {
        spdlog::error("Failed to create frame data buffer");
        return false;
    }

    visionary_control.reset(new visionary::VisionaryControl);
    if (!visionary_control)
    {
        spdlog::error("Failed to create camera control channel");
        return false;
    }

    if (!visionary_control->open(visionary::VisionaryControl::ProtocolType::COLA_2, ip, timeout_ms))
    {
        spdlog::error("Failed to open camera control channel");
        return false;
    }

    if (!visionary_control->stopAcquisition())
    {
        spdlog::error("Failed to stop frame acquisition");
        return false;
    }

    // start continous acquisition
    if (!visionary_control->startAcquisition())
    {
        spdlog::error("Failed to start frame acquisition");
        return false;
    }

    return true;
}

namespace ImGui
{
    void FrameWindow(const frame::Frame& frame, const char* name, bool* p_open, ImGuiWindowFlags flags)
    {
        // convert frame to texture

        ImGui::Begin(name, p_open, flags);
        {
            auto frame_mat = frame::to_mat(frame);
            int width, height;
            if (frame::to_texture(frame_mat, g_texture, width, height))
            {
                ImGui::Text("pointer = %p", g_texture);
                ImGui::Text("size = %d x %d", width, height);
                ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(g_texture)), ImVec2(width, height));
            }
            else
            {
                std::cerr << "failed to convert to texture\n";
            }
        }

        ImGui::End();
    }
}

void glfw_error_callback(int error, const char* description)
{
    spdlog::error("GLFW Error %d: %s", error, description);
}