#pragma once

#include <gui/filter_base.h>

class blur_filter : public filter_base
{
public:
	blur_filter();
	~blur_filter() override;

	std::unique_ptr<filter_base> clone() const override;
	const std::string type() const override { return "blur-filter"; };
	const bool apply(cv::Mat& mat) const override;
	const bool from_json(const nlohmann::json& filter) override;
	const nlohmann::json to_json() const override;

private:
	int size_x;
	int size_y;
};