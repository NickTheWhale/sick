#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <Framegrabber.h>
#include <VisionaryControl.h>
#include <VisionaryTMiniData.h>

#include <GL/glew.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <gui/frameset.h>
#include <gui/filter_pipeline.h>

#include <json.hpp>

#include <imnodes.h>
#include <gui/filter_editor.h>

bool mat_to_texture(const cv::Mat& mat, GLuint& texture, int& texture_width, int& texture_height);

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
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

    // Camera setup
    const std::string cam_ip = "192.168.1.67";
    visionary::FrameGrabber<visionary::VisionaryTMiniData> grabber(cam_ip, htons(2114u), 5000);
    std::shared_ptr<visionary::VisionaryTMiniData> data_handler;
    visionary::VisionaryControl visionary_control;

    if (!visionary_control.open(visionary::VisionaryControl::ProtocolType::COLA_2, cam_ip, 5000/*ms*/))
    {
        //std::cout << "Failed to open control connection to camera.\n";
        //return -1;
    }

    visionary_control.stopAcquisition();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // start continous acquisition
    if (!visionary_control.startAcquisition())
    {
        //std::cout << "Failed to start acquisition\n";
        //return -1;
    }

    
    filter_pipeline pipe;
    
    GLuint frame_texture = 0;
    glGenTextures(1, &frame_texture);

    editor::filter_editor feditor;
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        feditor.show();
        const bool pipeline_ok = feditor.create_pipeline(pipe);

        ImGui::ShowDemoWindow();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        // Get camera frame
        static int fn = 0;
        if (grabber.getCurrentFrame(data_handler))
        {
            fn = data_handler->getFrameNum();
        }
        ImGui::Begin("ave depth");
        if (data_handler->getDistanceMap().size() > 0)
        {
            size_t ave_depth = 0;
            for (const auto& val : data_handler->getDistanceMap())
                ave_depth += val;
            ave_depth /= data_handler->getDistanceMap().size();
            ImGui::Text("Average depth: %d", ave_depth);
            ImGui::End();
        }


        ImGui::Text("Frame number %d", fn);

        // Convert frame to texture
        frameset::Frame frame(data_handler->getDistanceMap(), data_handler->getHeight(), data_handler->getWidth(), data_handler->getFrameNum(), data_handler->getTimestamp());

        cv::Mat frame_mat = frameset::toMat(frame);
        int texture_width, texture_height;
        mat_to_texture(frame_mat, frame_texture, texture_width, texture_height);
        // TODO: seperate raw and filtered frame buffer pointers
        // TODO: resizable frame window
        // Display texture
        ImGui::Begin("Raw frame");
        ImGui::Text("pointer = %p", frame_texture);
        ImGui::Text("size = %d x %d", texture_width, texture_height);
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(frame_texture)), ImVec2(texture_width, texture_height));
        ImGui::End();

        if (pipeline_ok)
        {
            // TODO: filtering needs to be done in a seperate thread
            bool ret = pipe.apply(frame_mat);
            if (!ret)
                std::cerr << "failed to apply filter pipeline\n";
        }

        mat_to_texture(frame_mat, frame_texture, texture_width, texture_height);
        // Display texture
        ImGui::Begin("Filtered frame");
        ImGui::Text("pointer = %p", frame_texture);
        ImGui::Text("size = %d x %d", texture_width, texture_height);
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(frame_texture)), ImVec2(texture_width, texture_height));
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    imnodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool mat_to_texture(const cv::Mat& mat, GLuint& texture, int &texture_width, int &texture_height)
{
    if (mat.empty())
        return false;

    cv::Mat in_mat = mat;
    in_mat.convertTo(in_mat, CV_8U, 0.00390625);
    cv::cvtColor(in_mat, in_mat, cv::COLOR_GRAY2RGBA);

    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        in_mat.cols,
        in_mat.rows,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        in_mat.data);

    texture_width = in_mat.cols;
    texture_height = in_mat.rows;

    return true;
}
