#include "gui/frame_helper.h"

/**
 * @brief Convert the given cv::Mat to a opengl texture.
 * 
 * @param mat Input cv::Mat of type CV_16UC1 or CV_16UC3
 * @param texture Output texture
 * @return true if successful, false otherwise
 */
const bool frame::helper::to_texture(const cv::Mat& mat, GLuint& texture)
{
    if (mat.empty())
        return false;

    cv::Mat in_mat = mat;

    assert(in_mat.depth() == CV_16U);

    // convert to 8 bit RGBA format
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

    // convert to texture
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
