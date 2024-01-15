#include <algorithm>
#include <chrono>
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

//#define IMNODES_NAMESPACE imnodes
#include <imnodes.h>
#include <gui/filter_editor.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::unique_ptr<visionary::FrameGrabber<visionary::VisionaryTMiniData>> frame_grabber;
std::shared_ptr<visionary::VisionaryTMiniData> frame_data;
std::unique_ptr<visionary::VisionaryControl> visionary_control;
int g_octets[4] = {};
int g_port = {};
GLuint g_texture;

const bool open_camera(const std::string& ip, const uint16_t& port, const uint64_t& timeout_ms);

namespace ImGui
{
    void FrameWindow(const frame::Frame& frame, const char* name, bool *p_open = (bool *)0, ImGuiWindowFlags flags = 0);
    void CameraConnectionWindow(const char* name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
    void LogOptionsSelector(const char* name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
}

void glfw_error_callback(int error, const char* description);

int main(int, char**)
{
    // create loggers
    auto filter_logger = spdlog::stdout_color_mt("filter");
    auto ui_logger = spdlog::stdout_color_mt("ui");
    auto camera_logger = spdlog::stdout_color_mt("camera");

#ifdef _DEBUG
    filter_logger->set_level(spdlog::level::trace);
    ui_logger->set_level(spdlog::level::trace);
    camera_logger->set_level(spdlog::level::trace);
#endif // _DEBUG
   
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
    //glfwSwapInterval(1); // Enable vsync
    glfwSwapInterval(0);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();
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
        spdlog::get("ui")->error("GLEW failed to init: {}", reinterpret_cast<const char*>(glewGetErrorString(err)));
    }

    spdlog::get("ui")->debug("Using GLEW version {}", reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)));
    spdlog::get("ui")->debug("Using GL version {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    //open_camera("192.168.1.67", 2114, 1000);

    filter_pipeline pipeline;
    editor::filter_editor editor;
    filter::filter_worker worker;
    cv::Mat filtered_mat;
    frame::Frame filtered_frame;
    bool show_editor = false;
    glGenTextures(1, &g_texture);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        
        ImGui::LogOptionsSelector("Log Options");

        editor.show();
        ImGui::CameraConnectionWindow("Camera");
        
        bool pipeline_ok = editor.create_pipeline(pipeline);
        if (pipeline_ok)
        {
            if (frame_grabber && frame_grabber->getCurrentFrame(frame_data))
            {
                frame::Frame depth(
                    frame_data->getDistanceMap(),
                    frame_data->getHeight(),
                    frame_data->getWidth(),
                    frame_data->getFrameNum(),
                    frame_data->getTimestampMS()
                );

                auto depth_mat = frame::to_mat(depth);
                worker.set_pipeline(pipeline);
                worker.try_put_new(depth_mat);
            }
        }

        if (worker.try_latest_mat(filtered_mat) && !filtered_mat.empty())
            filtered_frame = frame::to_frame(filtered_mat);

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
    ImNodes::DestroyContext();
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
        spdlog::get("camera")->error("Failed to create frame grabber");
        return false;
    }

    frame_data.reset(new visionary::VisionaryTMiniData);
    if (!frame_data)
    {
        spdlog::get("camera")->error("Failed to create frame data buffer");
        return false;
    }

    visionary_control.reset(new visionary::VisionaryControl);
    if (!visionary_control)
    {
        spdlog::get("camera")->error("Failed to create camera control channel");
        return false;
    }

    if (!visionary_control->open(visionary::VisionaryControl::ProtocolType::COLA_2, ip, timeout_ms))
    {
        spdlog::get("camera")->error("Failed to open camera control channel");
        return false;
    }

    if (!visionary_control->stopAcquisition())
    {
        spdlog::get("camera")->error("Failed to stop frame acquisition");
        return false;
    }

    // start continous acquisition
    if (!visionary_control->startAcquisition())
    {
        spdlog::get("camera")->error("Failed to start frame acquisition");
        return false;
    }

    spdlog::get("camera")->info("Camera opened");

    return true;
}

namespace ImGui
{
    void FrameWindow(const frame::Frame& frame, const char* name, bool* p_open, ImGuiWindowFlags flags)
    {
        ImGui::Begin(name, p_open, flags);
        frame::Size frame_size = frame::size(frame);
        if (frame_size.height != 0 && frame_size.width != 0)
        {
            auto frame_mat = frame::to_mat(frame);
            int texture_width, texture_height;
            if (frame::to_texture(frame_mat, g_texture, texture_width, texture_height))
            {
                ImGui::Text("pointer = %p", g_texture);
                ImGui::Text("size = %d x %d", texture_width, texture_height);

                const ImVec2 available_size = ImGui::GetContentRegionAvail();
                const float frame_aspect = static_cast<float>(texture_width) / static_cast<float>(texture_height);

                ImVec2 texture_size = {
                    std::min(available_size.x, available_size.y * frame_aspect),
                    std::min(available_size.y, available_size.x / frame_aspect)
                };
                
                ImGui::SetCursorPos(ImGui::GetCursorPos() + (ImGui::GetContentRegionAvail() - texture_size) * 0.5f);
                ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(g_texture)), texture_size);
            }
            else
            {
                spdlog::get("ui")->error("Failed to convert frame to texture");
            }
        }

        ImGui::End();
    }

    void CameraConnectionWindow(const char* name, bool* p_open, ImGuiWindowFlags flags)
    {
        //ImGui::SetNextWindowPosCenter();
        ImGui::Begin(name, p_open, flags);

        float width = ImGui::CalcItemWidth();
        ImGui::BeginGroup();
        ImGui::PushID("IP");
        ImGui::TextUnformatted("IP");
        ImGui::SameLine();
        int i;
        for (i = 0; i < 4; i++) {
            ImGui::PushItemWidth(width / 5.0f);
            ImGui::PushID(i);
            bool invalid_octet = false;
            if (g_octets[i] > 255) {
                // Make values over 255 red, and when focus is lost reset it to 255.
                g_octets[i] = 255;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            if (g_octets[i] < 0) {
                // Make values below 0 yellow, and when focus is lost reset it to 0.
                g_octets[i] = 0;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
            ImGui::InputInt("##v", &g_octets[i], 0, 0, ImGuiInputTextFlags_CharsDecimal);
            if (invalid_octet) {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::PopItemWidth();
        }
        ImGui::PushItemWidth(width / 5.0f);
        ImGui::PushID("PORT");
        ImGui::TextUnformatted("Port");
        ImGui::SameLine();
        bool invalid_port = false;
        if (g_port > 65535)
        {
            g_port = 65535;
            invalid_port = true;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        }
        if (g_port < 0)
        {
            g_port = 0;
            invalid_port = true;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
        }
        ImGui::InputInt("##v", &g_port, 0, 0, ImGuiInputTextFlags_CharsDecimal);
        if (invalid_port)
            ImGui::PopStyleColor();
        ImGui::PopID();

        ImGui::PopID();
        ImGui::EndGroup();

        ImGui::SameLine();
        if (ImGui::Button("Connect")) 
        {
            std::stringstream ip;
            ip << g_octets[0] << "." << g_octets[1] << "." << g_octets[2] << "." << g_octets[3];
            open_camera(ip.str(), g_port, 5000);
        }
        ImGui::End();
    }
    
    void LogOptionsSelector(const char* name, bool* p_open, ImGuiWindowFlags flags)
    {
        ImGui::Begin(name, p_open, flags);

        constexpr int num_levels = 6;
        const char* levels[num_levels] = {
            spdlog::level::to_string_view(spdlog::level::trace).data(),
            spdlog::level::to_string_view(spdlog::level::debug).data(),
            spdlog::level::to_string_view(spdlog::level::info).data(),
            spdlog::level::to_string_view(spdlog::level::warn).data(),
            spdlog::level::to_string_view(spdlog::level::err).data(),
            spdlog::level::to_string_view(spdlog::level::critical).data()
        };

#ifdef _DEBUG
        constexpr int default_level = 0;
#else
        constexpr int default_level = 2;
#endif // _DEBUG


        static int filter_level = default_level;
        if (ImGui::Combo("Filter log level", &filter_level, levels, num_levels))
            spdlog::get("filter")->set_level(spdlog::level::from_str(levels[filter_level]));

        static int ui_level = default_level;
        if (ImGui::Combo("UI log level", &ui_level, levels, num_levels))
            spdlog::get("ui")->set_level(spdlog::level::from_str(levels[ui_level]));

        static int camera_level = default_level;
        if (ImGui::Combo("Camera log level", &camera_level, levels, num_levels))
            spdlog::get("camera")->set_level(spdlog::level::from_str(levels[camera_level]));

        ImGui::End();
    }
}

void glfw_error_callback(int error, const char* description)
{
    spdlog::get("ui")->error("GLFW Error {}: {}", error, description);
}