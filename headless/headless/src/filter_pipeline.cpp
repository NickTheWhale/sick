#include <headless/filter_pipeline.h>

#include <headless/filters/bilateral_filter.h>
#include <headless/filters/blur_filter.h>
#include <headless/filters/crop_filter.h>
#include <headless/filters/gaussian_blur_filter.h>
#include <headless/filters/median_filter.h>
#include <headless/filters/moving_average_filter.h>
#include <headless/filters/resize_filter.h>
#include <headless/filters/stack_blur_filter.h>
#include <headless/filters/threshold_filter.h>


filter_pipeline::filter_pipeline()
{
}

const bool filter_pipeline::from_json(const nlohmann::json& filters)
{
	for (const auto& filter_json : filters)
	{
		const std::string filter_type = filter_json["type"].get<std::string>();
		std::unique_ptr<filter_base> filter = make_filter(filter_type);
		if (filter)
		{
			filter->from_json(filter_json);
			this->filters.push_back(std::move(filter));
		}
	}

	return {};
}

const nlohmann::json filter_pipeline::to_json() const
{
	nlohmann::json::array_t filters;

	for (const auto& filter : this->filters)
	{
		filters.push_back(filter->to_json());
	}

	return filters;
}

const bool filter_pipeline::apply(cv::Mat& mat) const
{
	return false;
}

std::unique_ptr<filter_base> filter_pipeline::make_filter(const std::string& type) const
{
	if (type == bilateral_filter().type())
		return bilateral_filter().clone();

	if (type == blur_filter().type())
		return blur_filter().clone();

	if (type == crop_filter().type())
		return crop_filter().clone();

	if (type == gaussian_blur_filter().type())
		return gaussian_blur_filter().clone();

	if (type == median_filter().type())
		return median_filter().clone();

	if (type == moving_average_filter().type())
		return moving_average_filter().clone();

	if (type == resize_filter().type())
		return resize_filter().clone();

	if (type == stack_blur_filter().type())
		return stack_blur_filter().clone();

	if (type == threshold_filter().type())
		return threshold_filter().clone();


	return nullptr;
}
