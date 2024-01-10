#include <gui/filters/bilateral_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

bilateral_filter::bilateral_filter()
{
}

bilateral_filter::~bilateral_filter()
{
}

std::unique_ptr<filter_base> bilateral_filter::clone() const
{
	return std::make_unique<bilateral_filter>(*this);
}

const bool bilateral_filter::apply(cv::Mat& mat) const
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

const bool bilateral_filter::from_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		diameter = parameters["diameter"].get<int>();
		sigma_color = parameters["sigma-color"].get<float>();
		sigma_space = parameters["sigma-space"].get<float>();
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

const nlohmann::json bilateral_filter::to_json() const
{
	nlohmann::json root;
	try
	{
		nlohmann::json parameters;
		parameters["diameter"] = diameter.value();
		parameters["sigma-color"] = sigma_color.value();
		parameters["sigma-space"] = sigma_space.value();

		root["type"] = type();
		root["parameters"] = parameters;

		return root;
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

	return root;
}
