#include "gui/windows/camera_handler_window.h"

#include "spdlog/spdlog.h"

window::camera_handler_window::camera_handler_window(const char* name, bool* p_open, ImGuiWindowFlags flags)
	: window_base(name, p_open, flags), _octets(), _port(0)
{
}

window::camera_handler_window::~camera_handler_window()
{
    if (_visionary_control)
        _visionary_control->stopAcquisition();
}

const bool window::camera_handler_window::get_current_frame(frame::Frame& frame)
{
    if (!_frame_grabber)
        return false;

    if (!_frame_grabber->getCurrentFrame(_data_handler))
        return false;

    frame.data = _data_handler->getDistanceMap();
    frame.height = _data_handler->getHeight();
    frame.width = _data_handler->getWidth();
    frame.number = _data_handler->getFrameNum();
    frame.time_ms = _data_handler->getTimestampMS();

    return true;
}

void window::camera_handler_window::render_content()
{
    const float width = ImGui::CalcItemWidth();
    ImGui::BeginGroup();
    ImGui::PushID("IP");
    ImGui::TextUnformatted("IP");
    ImGui::SameLine();
    for (int i = 0; i < 4; i++) {
        ImGui::PushItemWidth(width / 5.0f);
        ImGui::PushID(i);
        bool invalid_octet = false;
        if (_octets[i] > 255) {
            // Make values over 255 red, and when focus is lost reset it to 255.
            _octets[i] = 255;
            invalid_octet = true;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        }
        if (_octets[i] < 0) {
            // Make values below 0 yellow, and when focus is lost reset it to 0.
            _octets[i] = 0;
            invalid_octet = true;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
        }
        ImGui::InputInt("##v", &_octets[i], 0, 0, ImGuiInputTextFlags_CharsDecimal);
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
    if (_port > 65535)
    {
        _port = 65535;
        invalid_port = true;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    }
    if (_port < 0)
    {
        _port = 0;
        invalid_port = true;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
    }
    ImGui::InputInt("##v", &_port, 0, 0, ImGuiInputTextFlags_CharsDecimal);
    if (invalid_port)
        ImGui::PopStyleColor();
    ImGui::PopID();

    ImGui::PopID();
    ImGui::EndGroup();

    ImGui::SameLine();
    if (ImGui::Button("Connect"))
    {
        std::stringstream ip;
        ip << _octets[0] << "." << _octets[1] << "." << _octets[2] << "." << _octets[3];
        open_camera(ip.str(), _port, 5000);
    }
}

const bool window::camera_handler_window::open_camera(const std::string& ip, const uint16_t& port, const uint32_t& timeout_ms)
{
    _frame_grabber.reset(new visionary::FrameGrabber<visionary::VisionaryTMiniData>(ip, htons(port), timeout_ms));
    if (!_frame_grabber)
    {
        spdlog::get("camera")->error("Failed to create frame grabber");
        return false;
    }

    _data_handler.reset(new visionary::VisionaryTMiniData);
    if (!_data_handler)
    {
        spdlog::get("camera")->error("Failed to create frame data buffer");
        return false;
    }

    _visionary_control.reset(new visionary::VisionaryControl);
    if (!_visionary_control)
    {
        spdlog::get("camera")->error("Failed to create camera control channel");
        return false;
    }

    if (!_visionary_control->open(visionary::VisionaryControl::ProtocolType::COLA_2, ip, timeout_ms))
    {
        spdlog::get("camera")->error("Failed to open camera control channel");
        return false;
    }

    if (!_visionary_control->stopAcquisition())
    {
        spdlog::get("camera")->error("Failed to stop frame acquisition");
        return false;
    }

    // start continuous acquisition
    if (!_visionary_control->startAcquisition())
    {
        spdlog::get("camera")->error("Failed to start frame acquisition");
        return false;
    }

    spdlog::get("camera")->info("Camera opened");

    return true;
}
