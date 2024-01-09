#pragma once

#include <gui/filter_base.h>

class gaussian_blur_filter : public filter_base
{
public:
	gaussian_blur_filter();
	~gaussian_blur_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "gaussian-blur-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool from_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	int size_x;
	int size_y;
};