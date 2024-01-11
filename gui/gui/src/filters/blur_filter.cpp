#include <gui/filters/blur_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

blur_filter::blur_filter()
{
}

blur_filter::~blur_filter()
{
}

std::unique_ptr<filter_base> blur_filter::clone() const
{
	return std::make_unique<blur_filter>(*this);
}

const bool blur_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;

	cv::blur(mat, output, cv::Size(size_x.value(), size_y.value()));
	mat = output;

	return true;
}

const bool blur_filter::load_json(const nlohmann::json& filter)
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

const nlohmann::json blur_filter::to_json() const
{
	nlohmann::json root;
	try
	{
		nlohmann::json parameters;
		parameters["kernel-size"]["x"] = size_x.value();
		parameters["kernel-size"]["y"] = size_y.value();

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
