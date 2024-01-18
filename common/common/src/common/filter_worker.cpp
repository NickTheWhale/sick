#include <gui/filter_worker.h>

#include <spdlog/spdlog.h>

filter::filter_worker::filter_worker()
	: _stop(false), _new_mat(false), _thread{&filter::filter_worker::run, this}
{
}

filter::filter_worker::~filter_worker()
{
	spdlog::get("filter")->debug("Stopping filter worker");
	_stop = true;
	_thread.join();
	spdlog::get("filter")->debug("Filter worker stopped");
}

void filter::filter_worker::set_pipeline(const filter_pipeline& pipeline)
{
	this->_pipeline = pipeline;
}

const filter::filter_pipeline filter::filter_worker::get_pipeline() const
{
	return _pipeline;
}

const bool filter::filter_worker::try_put_new(const cv::Mat& mat)
{
	std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);
	if (!locker.owns_lock())
		return false;

	_new_mat = true;
	mat.copyTo(_buffer);

	return true;
}

const bool filter::filter_worker::try_latest_mat(cv::Mat& mat) const
{
	std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);
	if (!locker.owns_lock())
		return false;
	
	_latest_mat.copyTo(mat);
	
	return true;
}

void filter::filter_worker::run()
{
	while (!_stop)
	{
		if (_new_mat)
		{
			_new_mat = false;
			{
				std::lock_guard<std::mutex> locker(_mutex);
				if (!_pipeline.apply(_buffer))
					spdlog::get("filter")->error("Filter worker failed to apply filters");
				_buffer.copyTo(_latest_mat);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}
