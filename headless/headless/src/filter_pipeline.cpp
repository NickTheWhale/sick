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

#include <headless/filter_factory.h>

#include <spdlog/spdlog.h>

filter::filter_pipeline::filter_pipeline(const filter_pipeline& other)
{
	for (const auto& filter : other.filters)
	{
		filters.push_back(filter->clone());
	}
}

filter::filter_pipeline::filter_pipeline(filter_pipeline&& other) noexcept
	: filters(std::move(other.filters))
{
}

filter::filter_pipeline& filter::filter_pipeline::operator=(const filter_pipeline& other)
{
	if (this != &other)
	{
		filters.clear();
		for (const auto& filter : other.filters)
		{
			filters.push_back(filter->clone());
		}
	}
	
	return *this;
}

const void filter::filter_pipeline::load_json(const nlohmann::json& filters)
{
	this->filters.clear();
	for (const auto& filter_json : filters)
	{
		if (!filter_json.contains("type"))
			continue;

		const std::string filter_type = filter_json["type"].get<std::string>();
		std::unique_ptr<filter_base> filter = filter::create(filter_type);
		if (filter)
		{
			filter->load_json(filter_json);
			this->filters.push_back(std::move(filter));
		}
	}
}

const nlohmann::json filter::filter_pipeline::to_json() const
{
	nlohmann::json filters;

	for (const auto& filter : this->filters)
	{
		filters.push_back(filter->to_json());
	}

	return filters;
}

const bool filter::filter_pipeline::apply(cv::Mat& mat) const
{
	try
	{
		for (const auto& filter : this->filters)
		{
			if (!filter->apply(mat))
				return false;
		}

		return true;
	}
	catch (const std::exception& e)
	{
		spdlog::get("filter")->error(e.what());

		return false;
	}
	catch (...)
	{
		spdlog::get("filter")->error("Filter pipeline failed to apply");

		return false;
	}
}
