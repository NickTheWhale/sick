#include "common/filters/blur_filter.h"

#include "opencv2/imgproc.hpp"

#include "spdlog/spdlog.h"

filter::blur_filter::blur_filter()
{
}

filter::blur_filter::~blur_filter()
{
}

std::unique_ptr<filter::filter_base> filter::blur_filter::clone() const
{
	return std::make_unique<filter::blur_filter>(*this);
}

const bool filter::blur_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;

	cv::blur(mat, output, cv::Size(size_x.value(), size_y.value()));
	mat = output;

	return true;
}

const bool filter::blur_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		size_x = parameters["kernel-size"]["x"].get<int>();
		size_y = parameters["kernel-size"]["y"].get<int>();
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

const nlohmann::json filter::blur_filter::to_json() const
{
	nlohmann::json root;
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"kernel-size", {
					{"x", size_x.value()},
					{"y", size_y.value()}
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