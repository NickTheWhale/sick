#include <gui/windows/frame_window.h>

#include <spdlog/spdlog.h>

namespace window
{
    frame_window::frame_window(const char* name, bool* p_open, ImGuiWindowFlags flags)
	    : window_base(name, p_open, flags), _need_to_generate(true), _texture(0)
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
        if (_need_to_generate)
            generate();

        ImGui::Text("pointer = %p", _texture);
        ImGui::Text("size = %d x %d", _frame.width, _frame.height);

        const ImVec2 available_size = ImGui::GetContentRegionAvail();
        const float frame_aspect = frame::aspect(_frame);

        ImVec2 texture_size = {
            std::min(available_size.x, available_size.y * frame_aspect),
            std::min(available_size.y, available_size.x / frame_aspect)
        };

        ImGui::SetCursorPos(ImGui::GetCursorPos() + (ImGui::GetContentRegionAvail() - texture_size) * 0.5f);
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(_texture)), texture_size);
    }

    const bool frame_window::generate()
    {
        const frame::Size frame_size = frame::size(_frame);
        if (frame_size.height == 0 || frame_size.width == 0)
            return false;

        _mat = frame::to_mat(_frame);
        int _;
        if (!frame::to_texture(_mat, _texture, _, _))
            return false;
    
        _need_to_generate = false;
        return true;
    }
}
