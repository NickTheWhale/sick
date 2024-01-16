#include <gui/filters/median_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

filter::median_filter::median_filter()
{
}

filter::median_filter::~median_filter()
{
}

std::unique_ptr<filter::filter_base> filter::median_filter::clone() const
{
	return std::make_unique<filter::median_filter>(*this);
}

const bool filter::median_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;
	cv::medianBlur(mat, output, size.value());

	mat = output;

	return true;
}

const bool filter::median_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		size = parameters["kernel-size"].get<int>();
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

const nlohmann::json filter::median_filter::to_json() const
{
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"kernel-size", size.value()},
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