#include "common/filters/resize_filter.h"

#include "opencv2/imgproc.hpp"

#include "spdlog/spdlog.h"

filter::resize_filter::resize_filter()
{
}

filter::resize_filter::~resize_filter()
{
}

std::unique_ptr<filter::filter_base> filter::resize_filter::clone() const
{
	return std::make_unique<resize_filter>(*this);
}

const bool filter::resize_filter::apply(cv::Mat& mat) const
{
	try
	{
		if (mat.empty())
			return false;

		cv::Mat output;
		cv::resize(mat, output, cv::Size(size_x.value(), size_y.value()), 0.0f, 0.0f, cv::InterpolationFlags::INTER_AREA);

		mat = output;
		return true;
	}
	catch (const cv::Exception& e)
	{
		spdlog::get("filter")->error("'{}' failed to apply with exception {}. Filter parameters:\n{}",
			type(), e.what(), to_json()["parameters"].dump(2));

		return false;
	}
}

const bool filter::resize_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];

		size_x = parameters["size"]["x"].get<int>();
		size_y = parameters["size"]["y"].get<int>();
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

const nlohmann::json filter::resize_filter::to_json() const
{
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"size", {
					{"x", size_x.value()},
					{"y", size_y.value()},
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