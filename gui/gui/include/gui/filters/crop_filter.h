#pragma once

#include <gui/filter_base.h>
#include <gui/filter_parameter.h>

class crop_filter : public filter_base
{
public:
	crop_filter();
	~crop_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "crop-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool load_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	filter::filter_parameter<double, 0.0, 1.0> center_x;
	filter::filter_parameter<double, 0.0, 1.0> center_y;

	filter::filter_parameter<double, 0.0, 1.0> width;
	filter::filter_parameter<double, 0.0, 1.0> height;
};