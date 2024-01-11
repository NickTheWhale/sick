#include <gui/filters/stack_blur_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

stack_blur_filter::stack_blur_filter()
	: size_x(size_x.min()), size_y(size_y.min())
{
}

stack_blur_filter::~stack_blur_filter()
{
}

std::unique_ptr<filter_base> stack_blur_filter::clone() const
{
	return std::make_unique<stack_blur_filter>(*this);
}

const bool stack_blur_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;
	cv::stackBlur(mat, output, cv::Size(size_x.value(), size_y.value()));

	mat = output;

	return true;
}

const bool stack_blur_filter::load_json(const nlohmann::json& filter)
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

const nlohmann::json stack_blur_filter::to_json() const
{
	try
	{
		nlohmann::json root;
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
