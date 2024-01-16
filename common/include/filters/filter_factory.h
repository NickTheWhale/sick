#pragma once

#include <string>
#include <unordered_map>

#include <gui/filter_base.h>

#include <gui/filters/bilateral_filter.h>
#include <gui/filters/blur_filter.h>
#include <gui/filters/crop_filter.h>
#include <gui/filters/gaussian_blur_filter.h>
#include <gui/filters/median_filter.h>
#include <gui/filters/moving_average_filter.h>
#include <gui/filters/resize_filter.h>
#include <gui/filters/stack_blur_filter.h>
#include <gui/filters/threshold_filter.h>

namespace filter
{
	const static std::unordered_map<std::string, std::function<std::unique_ptr<filter_base>()>> types = {
		{ bilateral_filter().type(),      []() { return bilateral_filter().clone(); }},
		{ blur_filter().type(),           []() { return blur_filter().clone(); }},
		{ crop_filter().type(),		      []() { return crop_filter().clone(); }},
		{ gaussian_blur_filter().type(),  []() { return gaussian_blur_filter().clone(); }},
		{ median_filter().type(),         []() { return median_filter().clone(); }},
		{ moving_average_filter().type(), []() { return moving_average_filter().clone(); }},
		{ resize_filter().type(),         []() { return resize_filter().clone(); }},
		{ stack_blur_filter().type(),     []() { return stack_blur_filter().clone(); }},
		{ threshold_filter().type(),      []() { return threshold_filter().clone(); }}
	};

	static std::unique_ptr<filter_base> create(const std::string& type)
	{
		auto it = types.find(type);
		if (it != types.end())
			return (*it).second();

		return nullptr;
	}
};


