#pragma once

#include <vector>

#include <json.hpp>

#include <gui/filter_base.h>

#include <opencv2/opencv.hpp>

class filter_pipeline
{
public:
	filter_pipeline();

	const bool from_json(const nlohmann::json& filters);
	const nlohmann::json to_json() const;
	const bool apply(cv::Mat& mat) const;

private:
	std::vector<std::unique_ptr<filter_base>> filters;

	std::unique_ptr<filter_base> make_filter(const std::string& type) const;
};

