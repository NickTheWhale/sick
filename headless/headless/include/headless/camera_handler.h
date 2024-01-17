#pragma once

#include <string>

#include <headless/frame.h>

#include <Framegrabber.h>	
#include <VisionaryControl.h>
#include <VisionaryTMiniData.h>

namespace camera
{
	class camera_handler
	{
	public:
		camera_handler();
		~camera_handler();

		const bool open(const std::string& ip, const uint16_t& port, const uint32_t& timeout_ms);
		const bool get_current_frame(frame::Frame& frame);
		const bool get_next_frame(frame::Frame& frame, const uint64_t timeout_ms = 1000);

	private:
		std::unique_ptr<visionary::FrameGrabber<visionary::VisionaryTMiniData>> _frame_grabber;
		std::shared_ptr<visionary::VisionaryTMiniData> _data_handler;
		std::unique_ptr<visionary::VisionaryControl> _visionary_control;
	};
}
