#include <gui/filter_worker.h>


filter::filter_worker::filter_worker()
	: _stop(false), _new_mat(false), _thread{&filter::filter_worker::run, this}
{
}

filter::filter_worker::~filter_worker()
{
	std::cout << "stopping worker\n";
	_stop = true;
	_thread.join();
	std::cout << "worker stopped\n";
}

void filter::filter_worker::set_pipeline(const filter_pipeline& pipeline)
{
	this->_pipeline = pipeline;
}

const filter_pipeline filter::filter_worker::get_pipeline() const
{
	return _pipeline;
}

void filter::filter_worker::put_new(const cv::Mat& mat)
{
	std::lock_guard<std::mutex> locker(_mutex);
	_new_mat = true;
	mat.copyTo(_buffer);
}

const cv::Mat filter::filter_worker::latest_mat() const
{
	std::lock_guard<std::mutex> locker(_mutex);
	return _latest_mat;
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
				_pipeline.apply(_buffer);
				_buffer.copyTo(_latest_mat);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}
