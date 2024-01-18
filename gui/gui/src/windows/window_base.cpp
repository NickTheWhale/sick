#include "gui/windows/window_base.h"

namespace window
{
	window_base::window_base(const char* name, bool* p_open, ImGuiWindowFlags flags)
		: window_name(name), p_open(p_open), window_flags(flags)
	{
	}

	window_base::~window_base()
	{
	}

	void window_base::render()
	{
		ImGui::Begin(window_name, p_open, window_flags);
		render_content();
		ImGui::End();
	}
}
