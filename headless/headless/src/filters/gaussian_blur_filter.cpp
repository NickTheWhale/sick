#include <headless/filters/gaussian_blur_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

gaussian_blur_filter::gaussian_blur_filter()
{
}

gaussian_blur_filter::~gaussian_blur_filter()
{
}

std::unique_ptr<filter_base> gaussian_blur_filter::clone() const
{
	return std::make_unique<gaussian_blur_filter>(*this);
}

const bool gaussian_blur_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	cv::Mat output;


	mat = output;
	return true;
}

const bool gaussian_blur_filter::from_json(const nlohmann::json& filter)
{
	try
	{

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

const nlohmann::json gaussian_blur_filter::to_json() const
{
	nlohmann::json root;
	try
	{

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
