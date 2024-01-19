#include "gui/windows/camera_handler_window.h"

#include "spdlog/spdlog.h"

window::camera_handler_window::camera_handler_window(const char* name, bool* p_open, ImGuiWindowFlags flags)
	: window_base(name, p_open, flags), _octets(), _port(0)
{
}

window::camera_handler_window::~camera_handler_window()
{
}

const bool window::camera_handler_window::get_current_frame(frame::Frame& frame)
{
    return _camera.get_current_frame(frame);
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
        _camera.open(ip.str(), static_cast<uint16_t>(_port), 5000);
    }
}