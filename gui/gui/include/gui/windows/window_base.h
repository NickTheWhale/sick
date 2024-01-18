#pragma once

#include "imgui.h"

namespace window
{
	class window_base
	{
	public:
		window_base(const char* name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
		virtual ~window_base() = 0;
		virtual void render();

	protected:
		virtual void render_content() = 0;

		const char* window_name;
		bool* p_open;
		ImGuiWindowFlags window_flags;
	};
}
