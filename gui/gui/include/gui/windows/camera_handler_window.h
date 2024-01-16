#pragma once

#include <gui/windows/window_base.h>
#include <gui/frame.h>

#include <Framegrabber.h>	
#include <VisionaryControl.h>
#include <VisionaryTMiniData.h>

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
		std::unique_ptr<visionary::FrameGrabber<visionary::VisionaryTMiniData>> _frame_grabber;
		std::shared_ptr<visionary::VisionaryTMiniData> _data_handler;
		std::unique_ptr<visionary::VisionaryControl> _visionary_control;

		int _octets[4];
		int _port;

		const bool open_camera(const std::string& ip, const uint16_t& port, const uint32_t& timeout_ms);
	};
}