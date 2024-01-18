#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

#include "GL/glew.h"

#include "opencv2/core/utils/logger.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "common/frame.h"
#include "common/filter_pipeline.h"
#include "common/filter_worker.h"

#include "json.hpp"

#include "imnodes.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "gui/windows/frame_window.h"
#include "gui/windows/filter_editor_window.h"
#include "gui/windows/camera_handler_window.h"

namespace ImGui
{
    void LogOptionsSelector(const char* name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
}

void glfw_error_callback(int error, const char* description);

int main(int, char**)
{
    // create loggers
    auto filter_logger = spdlog::stdout_color_mt("filter");
    auto camera_logger = spdlog::stdout_color_mt("camera");
    auto ui_logger = spdlog::stdout_color_mt("ui");
    spdlog::set_default_logger(ui_logger);

#ifdef _DEBUG
    filter_logger->set_level(spdlog::level::trace);
    ui_logger->set_level(spdlog::level::trace);
    camera_logger->set_level(spdlog::level::trace);
#endif // _DEBUG
   
#ifndef _DEBUG
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
#endif // _DEBUG

    glfwSetErrorCallback(glfw_error_callback);
    int ret = glfwInit();
    if (ret != GLFW_TRUE)
    {
        spdlog::critical("GLFW failed to initialize: {}", ret);
        return 1;
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Sick GUI", nullptr, nullptr);
    
    if (window == nullptr)
    {
        spdlog::critical("GLFW failed to create window");
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

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
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    
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

    filter::filter_pipeline pipeline;
    filter::filter_worker worker;
    cv::Mat filtered_mat;
    frame::Frame filtered_frame;
    window::frame_window filtered_frame_window("Filtered Frame");
    window::filter_editor_window editor_window("Filter Editor");
    window::camera_handler_window camera_handler_window("Camera Handler");
    while (!glfwWindowShouldClose(window))
    {
        //ImGuiViewport* viewport = ImGui::GetMainViewport();
        //ImGui::SetNextWindowPos(viewport->WorkPos);
        //ImGui::SetNextWindowSize(viewport->WorkSize);

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        
        ImGui::LogOptionsSelector("Log Options");

        editor_window.render();
        camera_handler_window.render();
        
        bool pipeline_ok = editor_window.create_pipeline(pipeline);
        if (pipeline_ok)
        {
            frame::Frame depth;
            if (camera_handler_window.get_current_frame(depth))
            {
                const cv::Mat& depth_mat = frame::to_mat(depth);
                worker.set_pipeline(pipeline);
                worker.try_put_new(depth_mat);
            }
        }

        if (worker.try_latest_mat(filtered_mat) && !filtered_mat.empty())
            filtered_frame = frame::to_frame(filtered_mat);

        filtered_frame_window.set_frame(filtered_frame);
        filtered_frame_window.render();

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

namespace ImGui
{    
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