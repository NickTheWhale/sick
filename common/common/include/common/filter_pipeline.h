#pragma once

#include <vector>
#include "json.hpp"
#include "common/include/common/filter_base.h"
#include "opencv2/opencv.hpp"

namespace filter
{
	class filter_pipeline
	{
	public:
		filter_pipeline() = default;
		filter_pipeline(const filter_pipeline& other);
		filter_pipeline(filter_pipeline&& other) noexcept;
		~filter_pipeline() = default;

		filter_pipeline& operator=(const filter_pipeline& other);

		const void load_json(const nlohmann::json& filters);
		const nlohmann::json to_json() const;
		const bool apply(cv::Mat& mat) const;
		const bool empty() const;

	private:
		std::vector<std::unique_ptr<filter_base>> filters;
	};
}
