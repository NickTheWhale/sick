#pragma once

#include "gui/windows/window_base.h"
#include "common/frame.h"
#include "opencv2/core/mat.hpp"
#include "GL/glew.h"

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
		bool _apply_colormap;
		cv::Mat _mat;
		frame::Frame _frame;
		GLuint _texture;
		static constexpr int num_colormaps = 22;
		static constexpr char const* colormap_names[num_colormaps] = {
			"AUTUMN",
			"BONE",
			"JET",
			"WINTER",
			"RAINBOW",
			"OCEAN",
			"SUMMER",
			"SPRING",
			"COOL",
			"HSV",
			"PINK",
			"HOT",
			"PARULA",
			"MAGMA",
			"INFERNO",
			"PLASMA",
			"VIRIDIS",
			"CIVIDIS",
			"TWILIGHT",
			"TWILIGHT_SHIFTED",
			"TURBO",
			"DEEPGREEN"
		};
		static constexpr cv::ColormapTypes colormap_types[num_colormaps] = {
			cv::COLORMAP_AUTUMN,
			cv::COLORMAP_BONE,
			cv::COLORMAP_JET,
			cv::COLORMAP_WINTER,
			cv::COLORMAP_RAINBOW,
			cv::COLORMAP_OCEAN,
			cv::COLORMAP_SUMMER,
			cv::COLORMAP_SPRING,
			cv::COLORMAP_COOL,
			cv::COLORMAP_HSV,
			cv::COLORMAP_PINK,
			cv::COLORMAP_HOT,
			cv::COLORMAP_PARULA,
			cv::COLORMAP_MAGMA,
			cv::COLORMAP_INFERNO,
			cv::COLORMAP_PLASMA,
			cv::COLORMAP_VIRIDIS,
			cv::COLORMAP_CIVIDIS,
			cv::COLORMAP_TWILIGHT,
			cv::COLORMAP_TWILIGHT_SHIFTED,
			cv::COLORMAP_TURBO,
			cv::COLORMAP_DEEPGREEN
		};
		
		const bool generate(const cv::ColormapTypes colormap);
	};
}
