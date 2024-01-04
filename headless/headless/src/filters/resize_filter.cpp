#include <headless/filters/resize_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

resize_filter::resize_filter() :
	size_x{10},
	size_y{10}
{
}

resize_filter::~resize_filter()
{
}

std::unique_ptr<filter_base> resize_filter::clone() const
{
	return std::make_unique<resize_filter>(*this);
}

const bool resize_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;
	cv::resize(mat, output, cv::Size(size_x, size_y), 0.0f, 0.0f, cv::InterpolationFlags::INTER_AREA);

	mat = output;
	return true;
}

const bool resize_filter::from_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		nlohmann::json size = parameters["size"];

		size_x = size["x"].get<int>();
		size_y = size["y"].get<int>();

		size_y = std::clamp(size_y, int(1), std::numeric_limits<int>::max());
		size_x = std::clamp(size_x, int(1), std::numeric_limits<int>::max());
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

const nlohmann::json resize_filter::to_json() const
{
	nlohmann::json root;
	try
	{
		nlohmann::json size;
		size["x"] = size_x;
		size["y"] = size_y;

		nlohmann::json parameters;
		parameters["size"] = size;

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
