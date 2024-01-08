/*****************************************************************//**
 * @file   Frameset.h
 * @brief  Defines frameset struct and some helper functions
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <vector>

#include <TinyColormap.hpp>

#include <opencv2/core.hpp>

namespace frameset
{
	/**
	 * @brief Defines how to apply color maps to depth frames.
	 */
	struct ImageOptions
	{
		tinycolormap::ColormapType colormap;
		// if autoExposure = false, exposureLow and exposureHigh will be used
		bool autoExposure;
		uint16_t exposureLow;
		uint16_t exposureHigh;
		// inverts the color
		bool invert;
		// treats input as logarithmic data. Useful for intensity map
		bool logarithmic;

		ImageOptions(
			const tinycolormap::ColormapType& colormap,
			bool autoExposure,
			const uint16_t& exposureLow,
			const uint16_t& exposureHigh,
			bool invert,
			bool logarithmic)
			: colormap(colormap), autoExposure(autoExposure), exposureLow(exposureLow), exposureHigh(exposureHigh), invert(invert), logarithmic(logarithmic)
		{}

		ImageOptions()
			: colormap(tinycolormap::ColormapType::Gray), autoExposure(true), exposureLow(std::numeric_limits<uint16_t>::min()), exposureHigh(std::numeric_limits<uint16_t>::max()), invert(false), logarithmic(false)
		{}

		bool operator==(const ImageOptions& other) const
		{
			return colormap == other.colormap && autoExposure == other.autoExposure && exposureLow == other.exposureLow && exposureHigh == other.exposureHigh && invert == other.invert && logarithmic == other.logarithmic;
		}
	};

	struct Frame
	{
		std::vector<uint16_t> data;
		uint32_t height;
		uint32_t width;
		uint32_t number;
		uint64_t time;

		Frame()
			: height(0), width(0), number(0), time(0)
		{
		}

		Frame(const std::vector<uint16_t>& data, const uint32_t height, const uint32_t width, const uint32_t number, const uint64_t time)
			: data(data), height(height), width(width), number(number), time(time)
		{
		}
	};

	struct Frameset
	{
		Frame depth;
		Frame intensity;
		Frame state;
	};
	
	// frame functions

	/**
	 * @brief Returns frame value at specified coordinate. No bounds checking!
	 */
	const uint16_t at(const Frame& frame, const uint32_t x, const uint32_t y);

	/**
	 * @brief Return frame size.
	 */
	//const QSize size(const Frame& frame);

	/**
	 * @brief Equivalent to size(&frame).isEmpty().
	 */
	//const bool isEmpty(const Frame& frame);

	/**
	 * @brief Returns true if frame data size equals frame.height * frame.width and frame is not empty.
	 */
	const bool isValid(const Frame& frame);

	/**
	 * @brief Sets all frame values outside of [lower, upper] to zero.
	 */
	void clip(Frame& frame, uint16_t lower, uint16_t upper);

	/**
	 * @brief Per-pixel std::clamp().
	 */
	void clamp(Frame& frame, uint16_t lower, uint16_t upper);

	/**
	 * @brief Any pixels outside of maskNorm rectangle are set to zero.
	 */
	//void mask(Frame& frame, const QRectF& maskNorm);

	/**
	 * @brief Converts frame to QImage using given ImageOptions.
	 */
	//const QImage toQImage(const Frame& frame, const ImageOptions& options);

	/**
	 * @brief Creates a Frame from given cv::Mat.
	 */
	const Frame toFrame(const cv::Mat& mat);

	/**
	 * @brief Creates a cv::Mat from give Frame.
	 */
	const cv::Mat toMat(const Frame& frame);

	/**
	 * @brief Per-pixel absolute difference. Input frames must have same size.
	 */
	//const Frame difference(const Frame& lhs, const Frame& rhs);

	// frameset functions

	/**
	 * @brief Returns true if all frames within frameset have the same dimensions.
	 */
	//const bool isUniform(const Frameset& fs);

	/**
	 * @brief Returns true if any frame within the given frameset is empty.
	 */
	//const bool isEmpty(const Frameset& fs);

	/**
	 * @brief Returns true if all frames within the given frameset are valid.
	 */
	//const bool isValid(const Frameset& fs);

	/**
	 * @brief Masks each frame within the given frameset.
	 */
	//void mask(Frameset& fs, const QRectF& maskNorm);
}

// needed (I think) for signals and slots
//Q_DECLARE_METATYPE(frameset::Frame);
//Q_DECLARE_METATYPE(frameset::Frameset);