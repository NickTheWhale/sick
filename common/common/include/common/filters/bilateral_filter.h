#pragma once

#include "common/include/common/filter_base.h"
#include "common/include/common/filter_parameter.h"

namespace filter
{
	class bilateral_filter : public filter_base
	{
	public:
		bilateral_filter();
		~bilateral_filter() override;

		std::unique_ptr<filter_base> clone() const override;
		const std::string type() const override { return "bilateral-filter"; };
		const bool apply(cv::Mat& mat) const override;
		const bool load_json(const nlohmann::json& filter) override;
		const nlohmann::json to_json() const override;

	private:
		filter::filter_parameter<int, 1, std::numeric_limits<int>::max()> diameter;
		filter::filter_parameter<double, 0.0, std::numeric_limits<double>::max()> sigma_color;
		filter::filter_parameter<double, 0.0, std::numeric_limits<double>::max()> sigma_space;
	};
}