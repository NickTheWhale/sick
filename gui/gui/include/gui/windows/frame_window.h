#pragma once

#include <gui/windows/window_base.h>
#include <gui/frame.h>
#include <opencv2/core/mat.hpp>

#include <GL/glew.h>

namespace window
{
	class frame_window : public window_base
	{
	public:
		frame_window(const char* name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
		~frame_window() override;
		void set_frame(const frame::Frame& frame);
	
	protected:
		void render_content() override;

	private:
		bool _need_to_generate;
		cv::Mat _mat;
		frame::Frame _frame;
		GLuint _texture;

		const bool generate();
	};
}
