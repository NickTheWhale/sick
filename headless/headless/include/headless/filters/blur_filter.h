#pragma once

#include <headless/filter_base.h>
#include <headless/filter_parameter.h>

namespace filter
{
	class blur_filter : public filter_base
	{
	public:
		blur_filter();
		~blur_filter() override;

		std::unique_ptr<filter_base> clone() const override;
		const std::string type() const override { return "blur-filter"; };
		const bool apply(cv::Mat& mat) const override;
		const bool load_json(const nlohmann::json& filter) override;
		const nlohmann::json to_json() const override;

	private:
		filter::filter_parameter<int, 1, std::numeric_limits<int>::max()> size_x;
		filter::filter_parameter<int, 1, std::numeric_limits<int>::max()> size_y;
	};
}
