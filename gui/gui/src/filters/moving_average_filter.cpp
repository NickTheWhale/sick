#include <gui/filters/moving_average_filter.h>

#include <opencv2/imgproc.hpp>

#include <spdlog/spdlog.h>

moving_average_filter::moving_average_filter()
{
}

moving_average_filter::~moving_average_filter()
{
}

std::unique_ptr<filter_base> moving_average_filter::clone() const
{
	return std::make_unique<moving_average_filter>(*this);
}

const bool moving_average_filter::apply(cv::Mat& mat) const
{
	if (mat.empty())
		return false;

	// add to buffer
	buffer.push_back(mat.clone());
	while (buffer.size() > buffer_size.value())
		buffer.pop_front();

	for (const cv::Mat& curr_mat : buffer)
		if (curr_mat.type() != mat.type() || curr_mat.size() != mat.size())
			return false;

	// average
	const cv::Mat first_mat = buffer.front();
	const cv::Size size = first_mat.size();
	cv::Mat accum_mat = cv::Mat::zeros(size, CV_64F);
	size_t num_mats = buffer.size();

	for (const cv::Mat& curr_mat : buffer)
	{
		cv::Mat mat_64F;
		curr_mat.convertTo(mat_64F, CV_64F);

		accum_mat += mat_64F;
	}

	const cv::Mat mean_mat = accum_mat / num_mats;

	cv::Mat output;
	mean_mat.convertTo(output, CV_16U);

	mat = output;

	return true;
}

const bool moving_average_filter::load_json(const nlohmann::json& filter)
{
	try
	{
		nlohmann::json parameters = filter["parameters"];
		buffer_size = parameters["buffer-size"].get<int>();
	}
	catch (const nlohmann::detail::exception& e)
	{
		spdlog::error("Failed to load '{}' filter from json: {}", type(), e.what());
		return false;
	}
	catch (...)
	{
		spdlog::error("Failed to load '{}' filter from json", type());
		return false;
	}

	return true;
}

const nlohmann::json moving_average_filter::to_json() const
{
	try
	{
		nlohmann::json j = {
			{"type", type()},
			{"parameters", {
				{"buffer-size", buffer_size.value()},
			}}
		};

		return j;
	}
	catch (const nlohmann::detail::exception& e)
	{
		spdlog::error("Failed to convert '{}' filter to json: {}", type(), e.what());
		return nlohmann::json{};
	}
	catch (...)
	{
		spdlog::error("Failed to convert '{}' filter to json", type());
		return nlohmann::json{};
	}
}