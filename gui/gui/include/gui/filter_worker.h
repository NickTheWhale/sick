#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include <opencv2/core/mat.hpp>

#include <gui/filter_pipeline.h>

namespace filter
{
	class filter_worker
	{
	public:
		filter_worker();
		~filter_worker();

		void put_new(const cv::Mat& mat);
		const cv::Mat latest_mat() const;

		void set_pipeline(const filter_pipeline& pipeline);
		const filter_pipeline get_pipeline() const;

	private:
		volatile bool _stop;

		mutable std::mutex _mutex;
		cv::Mat _latest_mat;

		std::atomic_bool _new_mat;
		cv::Mat _buffer;

		filter_pipeline _pipeline;

		std::thread _thread;

		void run();
	};
}
