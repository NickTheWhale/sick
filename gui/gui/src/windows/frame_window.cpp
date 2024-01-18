#include "gui/windows/frame_window.h"
#include "spdlog/spdlog.h"
#include "opencv2/imgproc.hpp"
#include "gui/frame_helper.h"

namespace window
{
    frame_window::frame_window(const char* name, bool* p_open, ImGuiWindowFlags flags)
	    : window_base(name, p_open, flags), _need_to_generate(true), _texture(0), _apply_colormap(true)
    {
	    glGenTextures(1, &_texture);
    }

    frame_window::~frame_window()
    {
	    glDeleteTextures(1, &_texture);
    }

    void frame_window::set_frame(const frame::Frame& frame)
    {
        if (this->_frame != frame)
        {
            _need_to_generate = true;
            this->_frame = frame;
        }
    }

    void frame_window::render_content()
    {
        static int colormap_index = 0;
        if (ImGui::Combo("Colormap", &colormap_index, colormap_names, num_colormaps))
            _need_to_generate = true;

        ImGui::SameLine();
        if (ImGui::Checkbox("Apply", &_apply_colormap))
            _need_to_generate = true;

        if (_need_to_generate)
            generate(colormap_types[colormap_index]);

        const ImVec2 available_size = ImGui::GetContentRegionAvail();
        const float frame_aspect = frame::aspect(_frame);

        ImVec2 texture_size = {
            std::min(available_size.x, available_size.y * frame_aspect),
            std::min(available_size.y, available_size.x / frame_aspect)
        };

        ImGui::SetCursorPos(ImGui::GetCursorPos() + (ImGui::GetContentRegionAvail() - texture_size) * 0.5f);
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(_texture)), texture_size);
    }

    const bool frame_window::generate(const cv::ColormapTypes colormap)
    {
        const frame::Size frame_size = frame::size(_frame);
        if (frame_size.height == 0 || frame_size.width == 0)
            return false;   

        _mat = frame::to_mat(_frame);

        if (_apply_colormap)
        {
            _mat.convertTo(_mat, CV_8UC1, 0.00390625);

            cv::applyColorMap(_mat, _mat, colormap);

            _mat.convertTo(_mat, CV_16UC1, 256.0);
        }

        if (!frame::helper::to_texture(_mat, _texture))
            return false;
    
        _need_to_generate = false;
        return true;
    }
}
