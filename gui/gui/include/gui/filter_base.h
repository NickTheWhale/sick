#pragma once

#include <string>

#include <opencv2/core/mat.hpp>

#include <json.hpp>

class filter_base
{
public:
	virtual ~filter_base() {};

	virtual std::unique_ptr<filter_base> clone() const = 0;
	virtual const std::string type() const = 0;
	virtual const bool apply(cv::Mat&) const = 0;
	virtual const bool from_json(const nlohmann::json& filter) = 0;
	virtual const nlohmann::json to_json() const = 0;
};

