#include <gui/frame.h>



const frame::Size frame::size(const Frame& frame)
{
    assert(frame.data.size() == frame.height * frame.width);

    return { .height = frame.height, .width = frame.width };
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
        assert(mat.channels() == 1 && mat.depth() == CV_16U, "Cannot convert mat with %d channels and %d depth", mat.channels(), mat.depth());
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

bool frame::to_texture(const cv::Mat& mat, GLuint& texture, int& texture_width, int& texture_height)
{
    if (mat.empty())
        return false;

    cv::Mat in_mat = mat;
    in_mat.convertTo(in_mat, CV_8U, 0.00390625);
    cv::cvtColor(in_mat, in_mat, cv::COLOR_GRAY2RGBA);

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

    texture_width = in_mat.cols;
    texture_height = in_mat.rows;

    return true;
}
