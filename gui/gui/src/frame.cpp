#include <gui/frame.h>

#include <limits>

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
    cv::Mat mat(frame.height, frame.width, CV_16UC1);
    for (uint32_t y = 0; y < frame.height; ++y)
    {
        for (uint32_t x = 0; x < frame.width; ++x)
        {
            mat.at<uint16_t>(cv::Point(x, y)) = frame.data[y * frame.width + x];
        }
    }

    assert(mat.type() == CV_16UC1);

    return mat;
}

const frame::Frame frame::to_frame(const cv::Mat& mat)
{
    Frame frame;

    assert(mat.type() == CV_16UC1);

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

bool frame::to_texture(const cv::Mat& mat, GLuint& texture)
{
    if (mat.empty())
        return false;

    cv::Mat in_mat = mat;

    assert(in_mat.depth() == CV_16U);

    switch (in_mat.channels())
    {
    case 1:
        in_mat.convertTo(in_mat, CV_8UC1, 0.00390625);
        cv::cvtColor(in_mat, in_mat, cv::COLOR_GRAY2RGBA);
        break;
    case 3:
        in_mat.convertTo(in_mat, CV_8UC3, 0.00390625);
        cv::cvtColor(in_mat, in_mat, cv::COLOR_RGB2RGBA);
        break;
    }


    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        in_mat.cols,
        in_mat.rows,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        in_mat.data);

    return true;
}