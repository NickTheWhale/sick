#pragma once

#include "filter_base.h"
#include "filter_parameter.h"

namespace filter
{
	class stack_blur_filter : public filter_base
	{
	public:
		stack_blur_filter();
		~stack_blur_filter() override;

		std::unique_ptr<filter_base> clone() const override;
		const std::string type() const override { return "stack-blur-filter"; };
		const bool apply(cv::Mat& mat) const override;
		const bool load_json(const nlohmann::json& filter) override;
		const nlohmann::json to_json() const override;

	private:
		filter::filter_parameter<int, 1, std::numeric_limits<int>::max(), true> size_x;
		filter::filter_parameter<int, 1, std::numeric_limits<int>::max(), true> size_y;
	};
}
