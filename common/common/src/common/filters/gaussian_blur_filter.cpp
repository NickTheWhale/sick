#include "common/filters/gaussian_blur_filter.h"

#include "opencv2/imgproc.hpp"

#include "spdlog/spdlog.h"

filter::gaussian_blur_filter::gaussian_blur_filter()
{
}

filter::gaussian_blur_filter::~gaussian_blur_filter()
{
}

std::unique_ptr<filter::filter_base> filter::gaussian_blur_filter::clone() const
{
	return std::make_unique<filter::gaussian_blur_filter>(*this);
}

const bool filter::gaussian_blur_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;
	cv::Size size(size_x.value(), size_y.value());
	cv::GaussianBlur(mat, output, size, sigma_x.value(), sigma_y.value());

	mat = output;
	return true;
}

const bool filter::gaussian_blur_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		size_x = parameters["kernel-size"]["x"].get<int>();
		size_y = parameters["kernel-size"]["y"].get<int>();
		sigma_x = parameters["sigma"]["x"].get<double>();
		sigma_y = parameters["sigma"]["y"].get<double>();
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

const nlohmann::json filter::gaussian_blur_filter::to_json() const
{
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"kernel-size", {
					{"x", size_x.value()},
					{"y", size_y.value()},
				}},
				{"sigma", {
					{"x", sigma_x.value()},
					{"y", sigma_y.value()},
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