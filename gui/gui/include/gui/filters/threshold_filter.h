#pragma once

#include <gui/filter_base.h>
#include <gui/filter_parameter.h>

namespace filter
{
	class threshold_filter : public filter_base
	{
	public:
		threshold_filter();
		~threshold_filter() override;

		std::unique_ptr<filter_base> clone() const override;
		const std::string type() const override { return "threshold-filter"; };
		const bool apply(cv::Mat& mat) const override;
		const bool load_json(const nlohmann::json& filter) override;
		const nlohmann::json to_json() const override;

	private:
		filter::filter_parameter<int, 0, std::numeric_limits<int>::max()> upper;
		filter::filter_parameter<int, 0, std::numeric_limits<int>::max()> lower;
	};
}
