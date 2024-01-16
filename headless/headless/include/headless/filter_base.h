#pragma once

#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>

#include <json.hpp>

#include <headless/filter_parameter.h>

namespace filter
{
	class filter_base
	{
	public:
		virtual ~filter_base() {};

		virtual std::unique_ptr<filter_base> clone() const = 0;
		virtual const std::string type() const = 0;
		virtual const bool apply(cv::Mat&) const = 0;
		virtual const bool load_json(const nlohmann::json& filter) = 0;
		virtual const nlohmann::json to_json() const = 0;
	};
}

