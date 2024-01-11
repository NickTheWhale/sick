#include <gui/filters/threshold_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

threshold_filter::threshold_filter()
	: upper(upper.max()), lower(lower.min())
{
}

threshold_filter::~threshold_filter()
{
}

std::unique_ptr<filter_base> threshold_filter::clone() const
{
	return std::make_unique<threshold_filter>(*this);
}

const bool threshold_filter::apply(cv::Mat& mat) const
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

const bool threshold_filter::load_json(const nlohmann::json& filter)
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

const nlohmann::json threshold_filter::to_json() const
{
	try
	{
		nlohmann::json root;
		nlohmann::json parameters;
		parameters["upper"] = upper.value();
		parameters["lower"] = lower.value();

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
}
