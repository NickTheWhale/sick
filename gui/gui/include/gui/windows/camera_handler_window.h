#pragma once

#include "gui/windows/window_base.h"
#include "common/frame.h"
#include "common/camera_handler.h"

#include "Framegrabber.h"
#include "VisionaryControl.h"
#include "VisionaryTMiniData.h"

namespace window
{
	class camera_handler_window : public window_base
	{
	public:
		camera_handler_window(const char* name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
		~camera_handler_window() override;

		const bool get_current_frame(frame::Frame& frame);

	protected:
		void render_content() override;

	private:
		int _octets[4];
		int _port;
		camera::camera_handler _camera;
	};
}