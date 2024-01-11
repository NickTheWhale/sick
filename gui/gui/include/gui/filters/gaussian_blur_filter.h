#pragma once

#include <gui/filter_base.h>
#include <gui/filter_parameter.h>

class gaussian_blur_filter : public filter_base
{
public:
	gaussian_blur_filter();
	~gaussian_blur_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "gaussian-blur-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool load_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	filter::filter_parameter<int, 1, std::numeric_limits<int>::max(), true> size_x;
	filter::filter_parameter<int, 1, std::numeric_limits<int>::max(), true> size_y;

	filter::filter_parameter<double, 0.0, std::numeric_limits<double>::max()> sigma_x;
	filter::filter_parameter<double, 0.0, std::numeric_limits<double>::max()> sigma_y;
};