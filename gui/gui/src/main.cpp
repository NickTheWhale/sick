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

GLFWwindow* glfw_window;

void glfw_error_callback(int error, const char* description);
void setup_logging();
int setup_imgui();
void cleanup_imgui();

int main(int, char**)
{
    // setup loggers. important to call this first as other files depend on the global logger registry
    //  containing the expected loggers (ex. "sickapi" logger)
    setup_logging();

    // setup all the necessary opengl, glfw, imgui stuff
    const int imgui_ok = setup_imgui();
    if (imgui_ok != 0)
    {
        spdlog::critical("Failed to setup ImGui: {}", imgui_ok);
        return imgui_ok;
    }

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    cv::Mat filtered_mat;
    frame::Frame filtered_frame;
    filter::filter_pipeline pipeline;
    filter::filter_worker worker;
    window::frame_window filtered_frame_window("Filtered Frame");
    window::filter_editor_window editor_window("Filter Editor");
    window::camera_handler_window camera_handler_window("Camera Handler");
    while (!glfwWindowShouldClose(glfw_window))
    {
        // required imgui things
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // application specific loop
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

        // required imgui things
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(glfw_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(glfw_window);
    }

    cleanup_imgui();

    return 0;
}

/**
 * @brief GLFW error callback.
 * 
 * Called if GLFW encounters an error and logs the error
 * 
 * @param error Error number
 * @param description Error description
 */
void glfw_error_callback(int error, const char* description)
{
    spdlog::get("ui")->error("GLFW Error {}: {}", error, description);
}

/**
 * @brief Creates logger objects and sets logging levels.
 * 
 */
void setup_logging()
{
    // create loggers
    auto filter_logger = spdlog::stdout_color_mt("filter");
    auto camera_logger = spdlog::stdout_color_mt("camera");
    auto ui_logger = spdlog::stdout_color_mt("ui");
    spdlog::set_default_logger(ui_logger);

    // set levels based on debug/release
#ifdef _DEBUG
    filter_logger->set_level(spdlog::level::trace);
    ui_logger->set_level(spdlog::level::trace);
    camera_logger->set_level(spdlog::level::trace);
#else
    filter_logger->set_level(spdlog::level::info);
    camera_logger->set_level(spdlog::level::info);
    ui_logger->set_level(spdlog::level::info);
#endif // _DEBUG

    // get rid of annoying opencv messages
#ifndef _DEBUG
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
#endif // _DEBUG
}

/**
 * @brief Setups ImGui, ImNodes, GLFW, and opengl contexts.
 * 
 * @return 0 on success, non-zero on error
 */
int setup_imgui()
{
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

    GLFWwindow* glfw_window = glfwCreateWindow(640, 480, "Sick GUI", nullptr, nullptr);

    if (glfw_window == nullptr)
    {
        spdlog::critical("GLFW failed to create window");
        return 1;
    }
    glfwMakeContextCurrent(glfw_window);
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
    ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        spdlog::get("ui")->error("GLEW failed to init: {}", reinterpret_cast<const char*>(glewGetErrorString(err)));
        return 1;
    }

    spdlog::get("ui")->debug("Using GLEW version {}", reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)));
    spdlog::get("ui")->debug("Using GL version {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    return 0;
}

/**
 * @brief Releases and opengl, GLFW, ImNodes, or ImGui resources.
 * 
 */
void cleanup_imgui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}