#include "common/filters/bilateral_filter.h"

#include "opencv2/imgproc.hpp"

#include "spdlog/spdlog.h"

filter::bilateral_filter::bilateral_filter()
	: diameter(1), sigma_color(0.0), sigma_space(0.0)
{
}

filter::bilateral_filter::~bilateral_filter()
{
}

std::unique_ptr<filter::filter_base> filter::bilateral_filter::clone() const
{
	return std::make_unique<bilateral_filter>(*this);
}

const bool filter::bilateral_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat input_32F;
	cv::Mat output_32F;
	cv::Mat output;
	mat.convertTo(input_32F, CV_32F);
	cv::bilateralFilter(input_32F, output_32F, diameter.value(), sigma_color.value(), sigma_space.value());
	output_32F.convertTo(output, CV_16U);
	mat = output;

	return true;
}

const bool filter::bilateral_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		diameter = parameters["diameter"].get<int>();
		sigma_color = parameters["sigma"]["color"].get<double>();
		sigma_space = parameters["sigma"]["space"].get<double>();
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

const nlohmann::json filter::bilateral_filter::to_json() const
{
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"diameter", diameter.value()},
				{"sigma", {
					{"color", sigma_color.value()},
					{"space", sigma_space.value()},
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