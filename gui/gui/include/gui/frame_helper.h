#pragma once

#include "opencv2/imgproc.hpp"
#include "GL/glew.h"

namespace frame
{
	namespace helper
	{
		const bool to_texture(const cv::Mat& mat, GLuint& texture);
	}
}
