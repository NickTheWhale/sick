#include <gui/filter_pipeline.h>

#include <gui/filters/bilateral_filter.h>
#include <gui/filters/blur_filter.h>
#include <gui/filters/crop_filter.h>
#include <gui/filters/gaussian_blur_filter.h>
#include <gui/filters/median_filter.h>
#include <gui/filters/moving_average_filter.h>
#include <gui/filters/resize_filter.h>
#include <gui/filters/stack_blur_filter.h>
#include <gui/filters/threshold_filter.h>

#include <gui/filter_factory.h>

filter_pipeline::filter_pipeline()
{
}

const void filter_pipeline::from_json(const nlohmann::json& filters)
{
	for (const auto& filter_json : filters)
	{
		const std::string filter_type = filter_json["type"].get<std::string>();
		std::unique_ptr<filter_base> filter = filter_factory::create(filter_type);
		if (filter)
		{
			filter->from_json(filter_json);
			this->filters.push_back(std::move(filter));
		}
	}
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
	for (const auto& filter : this->filters)
		if (!filter->apply(mat))
			return false;

	return true;
}
