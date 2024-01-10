#pragma once

#include <gui/filter_base.h>
#include <gui/filter_parameter.h>

class bilateral_filter : public filter_base
{
public:
	bilateral_filter();
	~bilateral_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "bilateral-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool from_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	filter::filter_parameter<int, 1, std::numeric_limits<int>::max(), false> diameter;
	filter::filter_parameter<float, 0.0f, std::numeric_limits<float>::max()> sigma_color;
	filter::filter_parameter<float, 0.0f, std::numeric_limits<float>::max()> sigma_space;
};
