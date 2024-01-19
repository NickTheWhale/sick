#include "common/filters/threshold_filter.h"

#include "opencv2/imgproc.hpp"

#include "spdlog/spdlog.h"

filter::threshold_filter::threshold_filter()
	: upper(upper.max()), lower(lower.min())
{
}

filter::threshold_filter::~threshold_filter()
{
}

std::unique_ptr<filter::filter_base> filter::threshold_filter::clone() const
{
	return std::make_unique<filter::threshold_filter>(*this);
}

const bool filter::threshold_filter::apply(cv::Mat& mat) const
{
	try
	{
		if (mat.empty())
			return false;

		cv::Mat output;
		cv::threshold(mat, output, upper.value(), 0, cv::THRESH_TOZERO_INV);
		mat = output;
    
		cv::threshold(mat, output, lower.value(), 0, cv::THRESH_TOZERO);
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

const bool filter::threshold_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		upper = parameters["upper"].get<int>();
		lower = parameters["lower"].get<int>();

		if (upper.value() < lower.value())
		{
			const auto upper_temp = upper;
			upper = lower.value();
			lower = upper_temp.value();
		}
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

const nlohmann::json filter::threshold_filter::to_json() const
{
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"upper", upper.value()},
				{"lower", lower.value()},
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