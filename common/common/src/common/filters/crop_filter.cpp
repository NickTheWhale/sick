#include "common/filters/crop_filter.h"

#include "opencv2/imgproc.hpp"

#include "spdlog/spdlog.h"

filter::crop_filter::crop_filter()
{
}

filter::crop_filter::~crop_filter()
{
}

std::unique_ptr<filter::filter_base> filter::crop_filter::clone() const
{
	return std::make_unique<crop_filter>(*this);
}

const bool filter::crop_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;

	// crop start

	int x = static_cast<int>(mat.cols * center_x.value() - width.value() * mat.cols / 2);
	int y = static_cast<int>(mat.rows * center_y.value() - height.value() * mat.rows / 2);
	int crop_width = static_cast<int>(mat.cols * width.value());
	int crop_height = static_cast<int>(mat.rows * height.value());

	// ensure the crop region is within the image bounds
	x = std::max(x, 0);
	y = std::max(y, 0);
	crop_width = std::min(crop_width, mat.cols - x);
	crop_height = std::min(crop_height, mat.rows - y);

	// limit smallest roi
	crop_width = std::max(1, crop_width);
	crop_height = std::max(1, crop_height);
	
	cv::Rect roi(x, y, crop_width, crop_height);
	output = mat(roi);


	// crop end

	mat = output;

	return true;
}

const bool filter::crop_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		center_x = parameters["center"]["x"].get<double>();
		center_y = parameters["center"]["y"].get<double>();
		width = parameters["size"]["width"].get<double>();
		height = parameters["size"]["height"].get<double>();
	}
	catch (const nlohmann::detail::exception& e)
	{
		spdlog::error("Failed to load '{}' filter from json: {}", type(), e.what());
		return false;
	}
	catch (...)
	{
		spdlog::error("Failed to load '{}' filter from json", type());
		return false;
	}

	return true;
}

const nlohmann::json filter::crop_filter::to_json() const
{
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"center", {
					{"x", center_x.value()},
					{"y", center_y.value()}
				}},
				{"size", {
					{"width", width.value()},
					{"height", height.value()},
				}}
			}}
		};

		return j;
	}
	catch (const nlohmann::detail::exception& e)
	{
		spdlog::error("Failed to convert '{}' filter to json: {}", type(), e.what());
		return nlohmann::json{};
	}
	catch (...)
	{
		spdlog::error("Failed to convert '{}' filter to json", type());
		return nlohmann::json{};
	}
}