#include <headless/frame.h>



const frame::Size frame::size(const Frame& frame)
{
    assert(frame.data.size() == frame.height * frame.width);

    return { .height = frame.height, .width = frame.width };
}

const float frame::aspect(const Frame& frame)
{
    if (frame.height == 0)
        return 0.0f;

    return static_cast<float>(frame.width) / static_cast<float>(frame.height);
}

const cv::Mat frame::to_mat(const Frame& frame)
{
    cv::Mat mat(frame.height, frame.width, CV_16U);
    for (uint32_t y = 0; y < frame.height; ++y)
    {
        for (uint32_t x = 0; x < frame.width; ++x)
        {
            mat.at<uint16_t>(cv::Point(x, y)) = frame.data[y * frame.width + x];
        }
    }

    return mat;
}

const frame::Frame frame::to_frame(const cv::Mat& mat)
{
    Frame frame;
    if (mat.channels() != 1 || mat.depth() != CV_16U)
    {
        assert(mat.channels() == 1 && mat.depth() == CV_16U);
        return frame;
    }

    frame.width = mat.cols;
    frame.height = mat.rows;
    frame.data.resize(frame.width * frame.height);

    for (uint32_t y = 0; y < frame.height; ++y)
    {
        for (uint32_t x = 0; x < frame.width; ++x)
        {
            frame.data[y * frame.width + x] = mat.at<uint16_t>(cv::Point(x, y));
        }
    }

    return frame;
}
