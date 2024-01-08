/*****************************************************************//**
 * @file   Frameset.cpp
 * @brief  Defines frameset struct and some helper functions
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include <gui/frameset.h>

namespace
{
	constexpr double linToLog(const double x)
	{
		/**
		* @brief Linear to logarithmic LUT.
		*
		* The table was generated using the following python code:
		* \code{.py}
		* # Import libraries
		* import matplotlib.pyplot as plt
		* import numpy as np
		*
		* # Creating vectors X and Y
		* x = np.linspace(0, 1, 1023)
		*
		* # Define f = log2(x + 1)
		* f = lambda x: np.log2(x + 1)
		*
		* # Number of applications of f(x)
		* iterations = 10
		*
		* y = f(x)
		* for i in range(iterations - 1):
		*     y = f(y)
		*
		* fig = plt.figure(figsize=(10, 5))
		* # Create the plot
		* plt.plot(x, y)
		*
		* # Write 'y' values to file
		* with open("lut.txt", "w") as file:
		*     for val in y:
		*         file.write(f"{val:.4f},\n")
		*
		* # Show the plot
		* plt.show()
		* \endcode
		*
		* @param x Linear input within [0.0, 1.0]
		* @return Logarithmic map within [0.0, 1.0]
		*/
		int index = x * 1023;

		if (index > 1023)
			index = 1023;
		else if (index < 0)
			index = 0;

		constexpr double data[] =
		{
			0.0000,
			0.0366,
			0.0705,
			0.1018,
			0.1309,
			0.1581,
			0.1835,
			0.2073,
			0.2296,
			0.2506,
			0.2705,
			0.2892,
			0.3070,
			0.3239,
			0.3399,
			0.3551,
			0.3696,
			0.3834,
			0.3966,
			0.4093,
			0.4214,
			0.4330,
			0.4441,
			0.4547,
			0.4650,
			0.4749,
			0.4844,
			0.4935,
			0.5023,
			0.5109,
			0.5191,
			0.5270,
			0.5347,
			0.5421,
			0.5493,
			0.5563,
			0.5630,
			0.5696,
			0.5759,
			0.5821,
			0.5881,
			0.5939,
			0.5996,
			0.6051,
			0.6104,
			0.6156,
			0.6207,
			0.6256,
			0.6304,
			0.6351,
			0.6397,
			0.6442,
			0.6485,
			0.6528,
			0.6569,
			0.6610,
			0.6649,
			0.6688,
			0.6726,
			0.6763,
			0.6799,
			0.6834,
			0.6869,
			0.6903,
			0.6936,
			0.6969,
			0.7001,
			0.7032,
			0.7062,
			0.7092,
			0.7122,
			0.7151,
			0.7179,
			0.7207,
			0.7234,
			0.7261,
			0.7287,
			0.7313,
			0.7338,
			0.7363,
			0.7387,
			0.7411,
			0.7435,
			0.7458,
			0.7481,
			0.7503,
			0.7526,
			0.7547,
			0.7569,
			0.7590,
			0.7610,
			0.7630,
			0.7650,
			0.7670,
			0.7690,
			0.7709,
			0.7727,
			0.7746,
			0.7764,
			0.7782,
			0.7800,
			0.7817,
			0.7834,
			0.7851,
			0.7868,
			0.7885,
			0.7901,
			0.7917,
			0.7933,
			0.7948,
			0.7963,
			0.7979,
			0.7994,
			0.8008,
			0.8023,
			0.8037,
			0.8051,
			0.8065,
			0.8079,
			0.8093,
			0.8106,
			0.8120,
			0.8133,
			0.8146,
			0.8158,
			0.8171,
			0.8184,
			0.8196,
			0.8208,
			0.8220,
			0.8232,
			0.8244,
			0.8255,
			0.8267,
			0.8278,
			0.8290,
			0.8301,
			0.8312,
			0.8322,
			0.8333,
			0.8344,
			0.8354,
			0.8365,
			0.8375,
			0.8385,
			0.8395,
			0.8405,
			0.8415,
			0.8425,
			0.8434,
			0.8444,
			0.8453,
			0.8463,
			0.8472,
			0.8481,
			0.8490,
			0.8499,
			0.8508,
			0.8516,
			0.8525,
			0.8534,
			0.8542,
			0.8551,
			0.8559,
			0.8567,
			0.8576,
			0.8584,
			0.8592,
			0.8600,
			0.8608,
			0.8615,
			0.8623,
			0.8631,
			0.8638,
			0.8646,
			0.8653,
			0.8661,
			0.8668,
			0.8675,
			0.8682,
			0.8690,
			0.8697,
			0.8704,
			0.8711,
			0.8717,
			0.8724,
			0.8731,
			0.8738,
			0.8744,
			0.8751,
			0.8757,
			0.8764,
			0.8770,
			0.8777,
			0.8783,
			0.8789,
			0.8795,
			0.8802,
			0.8808,
			0.8814,
			0.8820,
			0.8826,
			0.8832,
			0.8838,
			0.8843,
			0.8849,
			0.8855,
			0.8861,
			0.8866,
			0.8872,
			0.8877,
			0.8883,
			0.8888,
			0.8894,
			0.8899,
			0.8904,
			0.8910,
			0.8915,
			0.8920,
			0.8925,
			0.8931,
			0.8936,
			0.8941,
			0.8946,
			0.8951,
			0.8956,
			0.8961,
			0.8965,
			0.8970,
			0.8975,
			0.8980,
			0.8985,
			0.8989,
			0.8994,
			0.8999,
			0.9003,
			0.9008,
			0.9012,
			0.9017,
			0.9021,
			0.9026,
			0.9030,
			0.9035,
			0.9039,
			0.9043,
			0.9048,
			0.9052,
			0.9056,
			0.9060,
			0.9065,
			0.9069,
			0.9073,
			0.9077,
			0.9081,
			0.9085,
			0.9089,
			0.9093,
			0.9097,
			0.9101,
			0.9105,
			0.9109,
			0.9113,
			0.9117,
			0.9120,
			0.9124,
			0.9128,
			0.9132,
			0.9136,
			0.9139,
			0.9143,
			0.9147,
			0.9150,
			0.9154,
			0.9157,
			0.9161,
			0.9165,
			0.9168,
			0.9172,
			0.9175,
			0.9179,
			0.9182,
			0.9185,
			0.9189,
			0.9192,
			0.9196,
			0.9199,
			0.9202,
			0.9206,
			0.9209,
			0.9212,
			0.9215,
			0.9219,
			0.9222,
			0.9225,
			0.9228,
			0.9231,
			0.9234,
			0.9238,
			0.9241,
			0.9244,
			0.9247,
			0.9250,
			0.9253,
			0.9256,
			0.9259,
			0.9262,
			0.9265,
			0.9268,
			0.9271,
			0.9274,
			0.9277,
			0.9279,
			0.9282,
			0.9285,
			0.9288,
			0.9291,
			0.9294,
			0.9296,
			0.9299,
			0.9302,
			0.9305,
			0.9308,
			0.9310,
			0.9313,
			0.9316,
			0.9318,
			0.9321,
			0.9324,
			0.9326,
			0.9329,
			0.9332,
			0.9334,
			0.9337,
			0.9339,
			0.9342,
			0.9344,
			0.9347,
			0.9349,
			0.9352,
			0.9354,
			0.9357,
			0.9359,
			0.9362,
			0.9364,
			0.9367,
			0.9369,
			0.9372,
			0.9374,
			0.9376,
			0.9379,
			0.9381,
			0.9384,
			0.9386,
			0.9388,
			0.9390,
			0.9393,
			0.9395,
			0.9397,
			0.9400,
			0.9402,
			0.9404,
			0.9406,
			0.9409,
			0.9411,
			0.9413,
			0.9415,
			0.9417,
			0.9420,
			0.9422,
			0.9424,
			0.9426,
			0.9428,
			0.9430,
			0.9433,
			0.9435,
			0.9437,
			0.9439,
			0.9441,
			0.9443,
			0.9445,
			0.9447,
			0.9449,
			0.9451,
			0.9453,
			0.9455,
			0.9457,
			0.9459,
			0.9461,
			0.9463,
			0.9465,
			0.9467,
			0.9469,
			0.9471,
			0.9473,
			0.9475,
			0.9477,
			0.9479,
			0.9481,
			0.9483,
			0.9485,
			0.9486,
			0.9488,
			0.9490,
			0.9492,
			0.9494,
			0.9496,
			0.9498,
			0.9499,
			0.9501,
			0.9503,
			0.9505,
			0.9507,
			0.9508,
			0.9510,
			0.9512,
			0.9514,
			0.9516,
			0.9517,
			0.9519,
			0.9521,
			0.9523,
			0.9524,
			0.9526,
			0.9528,
			0.9529,
			0.9531,
			0.9533,
			0.9534,
			0.9536,
			0.9538,
			0.9539,
			0.9541,
			0.9543,
			0.9544,
			0.9546,
			0.9548,
			0.9549,
			0.9551,
			0.9553,
			0.9554,
			0.9556,
			0.9557,
			0.9559,
			0.9561,
			0.9562,
			0.9564,
			0.9565,
			0.9567,
			0.9568,
			0.9570,
			0.9571,
			0.9573,
			0.9575,
			0.9576,
			0.9578,
			0.9579,
			0.9581,
			0.9582,
			0.9584,
			0.9585,
			0.9587,
			0.9588,
			0.9589,
			0.9591,
			0.9592,
			0.9594,
			0.9595,
			0.9597,
			0.9598,
			0.9600,
			0.9601,
			0.9602,
			0.9604,
			0.9605,
			0.9607,
			0.9608,
			0.9610,
			0.9611,
			0.9612,
			0.9614,
			0.9615,
			0.9616,
			0.9618,
			0.9619,
			0.9621,
			0.9622,
			0.9623,
			0.9625,
			0.9626,
			0.9627,
			0.9629,
			0.9630,
			0.9631,
			0.9633,
			0.9634,
			0.9635,
			0.9636,
			0.9638,
			0.9639,
			0.9640,
			0.9642,
			0.9643,
			0.9644,
			0.9645,
			0.9647,
			0.9648,
			0.9649,
			0.9651,
			0.9652,
			0.9653,
			0.9654,
			0.9656,
			0.9657,
			0.9658,
			0.9659,
			0.9660,
			0.9662,
			0.9663,
			0.9664,
			0.9665,
			0.9666,
			0.9668,
			0.9669,
			0.9670,
			0.9671,
			0.9672,
			0.9674,
			0.9675,
			0.9676,
			0.9677,
			0.9678,
			0.9679,
			0.9681,
			0.9682,
			0.9683,
			0.9684,
			0.9685,
			0.9686,
			0.9687,
			0.9689,
			0.9690,
			0.9691,
			0.9692,
			0.9693,
			0.9694,
			0.9695,
			0.9696,
			0.9697,
			0.9699,
			0.9700,
			0.9701,
			0.9702,
			0.9703,
			0.9704,
			0.9705,
			0.9706,
			0.9707,
			0.9708,
			0.9709,
			0.9710,
			0.9711,
			0.9713,
			0.9714,
			0.9715,
			0.9716,
			0.9717,
			0.9718,
			0.9719,
			0.9720,
			0.9721,
			0.9722,
			0.9723,
			0.9724,
			0.9725,
			0.9726,
			0.9727,
			0.9728,
			0.9729,
			0.9730,
			0.9731,
			0.9732,
			0.9733,
			0.9734,
			0.9735,
			0.9736,
			0.9737,
			0.9738,
			0.9739,
			0.9740,
			0.9741,
			0.9742,
			0.9743,
			0.9744,
			0.9745,
			0.9746,
			0.9747,
			0.9748,
			0.9748,
			0.9749,
			0.9750,
			0.9751,
			0.9752,
			0.9753,
			0.9754,
			0.9755,
			0.9756,
			0.9757,
			0.9758,
			0.9759,
			0.9760,
			0.9761,
			0.9761,
			0.9762,
			0.9763,
			0.9764,
			0.9765,
			0.9766,
			0.9767,
			0.9768,
			0.9769,
			0.9770,
			0.9770,
			0.9771,
			0.9772,
			0.9773,
			0.9774,
			0.9775,
			0.9776,
			0.9777,
			0.9777,
			0.9778,
			0.9779,
			0.9780,
			0.9781,
			0.9782,
			0.9783,
			0.9783,
			0.9784,
			0.9785,
			0.9786,
			0.9787,
			0.9788,
			0.9788,
			0.9789,
			0.9790,
			0.9791,
			0.9792,
			0.9793,
			0.9793,
			0.9794,
			0.9795,
			0.9796,
			0.9797,
			0.9797,
			0.9798,
			0.9799,
			0.9800,
			0.9801,
			0.9801,
			0.9802,
			0.9803,
			0.9804,
			0.9805,
			0.9805,
			0.9806,
			0.9807,
			0.9808,
			0.9809,
			0.9809,
			0.9810,
			0.9811,
			0.9812,
			0.9812,
			0.9813,
			0.9814,
			0.9815,
			0.9816,
			0.9816,
			0.9817,
			0.9818,
			0.9819,
			0.9819,
			0.9820,
			0.9821,
			0.9822,
			0.9822,
			0.9823,
			0.9824,
			0.9825,
			0.9825,
			0.9826,
			0.9827,
			0.9828,
			0.9828,
			0.9829,
			0.9830,
			0.9830,
			0.9831,
			0.9832,
			0.9833,
			0.9833,
			0.9834,
			0.9835,
			0.9835,
			0.9836,
			0.9837,
			0.9838,
			0.9838,
			0.9839,
			0.9840,
			0.9840,
			0.9841,
			0.9842,
			0.9843,
			0.9843,
			0.9844,
			0.9845,
			0.9845,
			0.9846,
			0.9847,
			0.9847,
			0.9848,
			0.9849,
			0.9849,
			0.9850,
			0.9851,
			0.9851,
			0.9852,
			0.9853,
			0.9853,
			0.9854,
			0.9855,
			0.9855,
			0.9856,
			0.9857,
			0.9857,
			0.9858,
			0.9859,
			0.9859,
			0.9860,
			0.9861,
			0.9861,
			0.9862,
			0.9863,
			0.9863,
			0.9864,
			0.9865,
			0.9865,
			0.9866,
			0.9867,
			0.9867,
			0.9868,
			0.9868,
			0.9869,
			0.9870,
			0.9870,
			0.9871,
			0.9872,
			0.9872,
			0.9873,
			0.9873,
			0.9874,
			0.9875,
			0.9875,
			0.9876,
			0.9877,
			0.9877,
			0.9878,
			0.9878,
			0.9879,
			0.9880,
			0.9880,
			0.9881,
			0.9881,
			0.9882,
			0.9883,
			0.9883,
			0.9884,
			0.9884,
			0.9885,
			0.9886,
			0.9886,
			0.9887,
			0.9887,
			0.9888,
			0.9889,
			0.9889,
			0.9890,
			0.9890,
			0.9891,
			0.9892,
			0.9892,
			0.9893,
			0.9893,
			0.9894,
			0.9894,
			0.9895,
			0.9896,
			0.9896,
			0.9897,
			0.9897,
			0.9898,
			0.9898,
			0.9899,
			0.9900,
			0.9900,
			0.9901,
			0.9901,
			0.9902,
			0.9902,
			0.9903,
			0.9903,
			0.9904,
			0.9905,
			0.9905,
			0.9906,
			0.9906,
			0.9907,
			0.9907,
			0.9908,
			0.9908,
			0.9909,
			0.9910,
			0.9910,
			0.9911,
			0.9911,
			0.9912,
			0.9912,
			0.9913,
			0.9913,
			0.9914,
			0.9914,
			0.9915,
			0.9915,
			0.9916,
			0.9917,
			0.9917,
			0.9918,
			0.9918,
			0.9919,
			0.9919,
			0.9920,
			0.9920,
			0.9921,
			0.9921,
			0.9922,
			0.9922,
			0.9923,
			0.9923,
			0.9924,
			0.9924,
			0.9925,
			0.9925,
			0.9926,
			0.9926,
			0.9927,
			0.9927,
			0.9928,
			0.9928,
			0.9929,
			0.9929,
			0.9930,
			0.9930,
			0.9931,
			0.9931,
			0.9932,
			0.9932,
			0.9933,
			0.9933,
			0.9934,
			0.9934,
			0.9935,
			0.9935,
			0.9936,
			0.9936,
			0.9937,
			0.9937,
			0.9938,
			0.9938,
			0.9939,
			0.9939,
			0.9940,
			0.9940,
			0.9941,
			0.9941,
			0.9942,
			0.9942,
			0.9943,
			0.9943,
			0.9944,
			0.9944,
			0.9945,
			0.9945,
			0.9946,
			0.9946,
			0.9946,
			0.9947,
			0.9947,
			0.9948,
			0.9948,
			0.9949,
			0.9949,
			0.9950,
			0.9950,
			0.9951,
			0.9951,
			0.9952,
			0.9952,
			0.9952,
			0.9953,
			0.9953,
			0.9954,
			0.9954,
			0.9955,
			0.9955,
			0.9956,
			0.9956,
			0.9957,
			0.9957,
			0.9957,
			0.9958,
			0.9958,
			0.9959,
			0.9959,
			0.9960,
			0.9960,
			0.9961,
			0.9961,
			0.9961,
			0.9962,
			0.9962,
			0.9963,
			0.9963,
			0.9964,
			0.9964,
			0.9965,
			0.9965,
			0.9965,
			0.9966,
			0.9966,
			0.9967,
			0.9967,
			0.9968,
			0.9968,
			0.9968,
			0.9969,
			0.9969,
			0.9970,
			0.9970,
			0.9971,
			0.9971,
			0.9971,
			0.9972,
			0.9972,
			0.9973,
			0.9973,
			0.9974,
			0.9974,
			0.9974,
			0.9975,
			0.9975,
			0.9976,
			0.9976,
			0.9976,
			0.9977,
			0.9977,
			0.9978,
			0.9978,
			0.9978,
			0.9979,
			0.9979,
			0.9980,
			0.9980,
			0.9981,
			0.9981,
			0.9981,
			0.9982,
			0.9982,
			0.9983,
			0.9983,
			0.9983,
			0.9984,
			0.9984,
			0.9985,
			0.9985,
			0.9985,
			0.9986,
			0.9986,
			0.9987,
			0.9987,
			0.9987,
			0.9988,
			0.9988,
			0.9989,
			0.9989,
			0.9989,
			0.9990,
			0.9990,
			0.9990,
			0.9991,
			0.9991,
			0.9992,
			0.9992,
			0.9992,
			0.9993,
			0.9993,
			0.9994,
			0.9994,
			0.9994,
			0.9995,
			0.9995,
			0.9995,
			0.9996,
			0.9996,
			0.9997,
			0.9997,
			0.9997,
			0.9998,
			0.9998,
			0.9999,
			0.9999,
			0.9999,
			1.0000,
			1.0000
		};
		return data[index];
	}
}

//const uint16_t frameset::at(const Frame& frame, const uint32_t x, const uint32_t y)
//{
//	Q_ASSERT_X(x < frame.width && y < frame.height, __FUNCTION__, "(x, y) must be inside the frame");
//	return frame.data[y * frame.width + x];
//}

//const QSize frameset::size(const Frame& frame)
//{
//	return QSize(frame.width, frame.height);
//}

//const bool frameset::isEmpty(const Frame& frame)
//{
//	return size(frame).isEmpty();
//}

//const bool frameset::isValid(const Frame& frame)
//{
//	return frame.data.size() == frame.height * frame.width && !isEmpty(frame);
//}

void frameset::clip(Frame& frame, uint16_t lower, uint16_t upper)
{
	if (upper < lower)
	{
		std::swap(upper, lower);
	}
	for (uint16_t& val : frame.data)
	{
		if (val > upper)
			val = 0;
		else if (val < lower)
			val = 0;
	}
}

void frameset::clamp(Frame& frame, uint16_t lower, uint16_t upper)
{
	if (upper < lower)
	{
		std::swap(upper, lower);
	}
	for (uint16_t& val : frame.data)
	{
		val = std::clamp(val, lower, upper);
	}
}

//void frameset::mask(Frame& frame, const QRectF& maskNorm)
//{
//	if (maskNorm.isEmpty())
//		return;
//
//	if (size(frame).isEmpty())
//		return;
//
//	// "unnormalize" mask to frame size
//	const QPoint maskTopLeft = QPoint(maskNorm.topLeft().x() * frame.width, maskNorm.topLeft().y() * frame.height);
//	const QPoint maskBottomRight = QPoint(maskNorm.bottomRight().x() * frame.width, maskNorm.bottomRight().y() * frame.height) - QPoint(1, 1);
//
//	// "normalized" in this context does not mean [0, 1]. Read https://doc.qt.io/qt-6/qrect.html#normalized
//	const QRect mask = QRect(maskTopLeft, maskBottomRight).normalized();
//
//	// if the mask has zero size, dont bother continuing
//	if (mask.size().isEmpty())
//		return;
//
//	const size_t maskedWidth = mask.width();
//	const size_t maskedHeight = mask.height();
//
//	const std::vector<uint16_t> bufferCopy(frame.data);
//	frame.data.clear();
//
//	// iterate through mask coordinates and copy to buffer
//	for (size_t y = maskTopLeft.y(); y < maskTopLeft.y() + maskedHeight; ++y)
//	{
//		for (size_t x = maskTopLeft.x(); x < maskTopLeft.x() + maskedWidth; ++x)
//		{
//			const int index = y * frame.width + x;
//			Q_ASSERT_X(index < bufferCopy.size(), __FUNCTION__, "index out of range of buffer");
//			frame.data.push_back(bufferCopy[index]);
//		}
//	}
//
//	// update height and width
//	frame.height = maskedHeight;
//	frame.width = maskedWidth;
//
//	Q_ASSERT_X(frame.data.size() == frame.height * frame.width, __FUNCTION__, "data size does not match height * width");
//}

//const QImage frameset::toQImage(const Frame& frame, const ImageOptions& options)
//{
//	QImage image = QImage(frame.width, frame.height, QImage::Format_ARGB32_Premultiplied);
//
//	if (frame.data.empty())
//		return image;
//
//	// exposure limits
//	uint16_t exposureMin;
//	uint16_t exposureMax;
//
//	// set exposureMin and exposureMax to lowest non zero value and highest value, respectively
//	if (options.autoExposure)
//	{
//		exposureMax = std::numeric_limits<uint16_t>::min();
//		exposureMin = std::numeric_limits<uint16_t>::max();
//		for (const uint16_t& val : frame.data)
//		{
//			if (val > 0 && val < exposureMin)
//				exposureMin = val;
//			if (val > exposureMax)
//				exposureMax = val;
//		}
//	}
//	// or use specified limits
//	else
//	{
//		exposureMin = options.exposureLow;
//		exposureMax = options.exposureHigh;
//	}
//	// just in case
//	if (exposureMin > exposureMax)
//		std::swap(exposureMin, exposureMax);
//
//	const double delta = exposureMax - exposureMin;
//
//	// iterate through frame data, convert pixel value to a color, and assign to image
//	QRgb* imagePtr = reinterpret_cast<QRgb*>(image.bits());
//	for (size_t y = 0; y < frame.height; ++y)
//	{
//		for (size_t x = 0; x < frame.width; ++x)
//		{
//			const uint16_t val = frame.data[y * frame.width + x];
//			double valNorm = 0;
//			if (delta > 0)
//			{
//				// normalize raw pixel value 
//				valNorm = (val - exposureMin) / delta;
//				if (options.logarithmic)
//					valNorm = linToLog(valNorm);
//			}
//			
//			// calculate pixel color from normalized value, inverting if required
//			tinycolormap::Color color = options.invert ? tinycolormap::Color(255, 255, 255) - tinycolormap::GetColor(valNorm, options.colormap) : tinycolormap::GetColor(valNorm, options.colormap);
//			// assign image pixel color
//			imagePtr[y * frame.width + x] = qRgba((valNorm > 0) * color.ri(), (valNorm > 0) * color.gi(), (valNorm > 0) * color.bi(), (valNorm > 0) * 255);
//		}
//	}
//	return image;
//}

const frameset::Frame frameset::toFrame(const cv::Mat& mat)
{
	Frame frame;
	if (mat.channels() != 1 || mat.depth() != CV_16U)
		return frame;

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

const cv::Mat frameset::toMat(const Frame& frame)
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

//const frameset::Frame frameset::difference(const Frame& lhs, const Frame& rhs)
//{
//	Q_ASSERT_X(lhs.data.size() == rhs.data.size(), __FUNCTION__, "frames must have same sizes");
//	Q_ASSERT_X(size(lhs) == size(rhs), __FUNCTION__, "frames must have same sizes");
//	Frame diff;
//	
//	// per-pixel absolute difference
//	for (size_t i = 0; i < lhs.data.size(); ++i)
//	{
//		int32_t delta = lhs.data[i] - rhs.data[i];
//		diff.data.push_back(static_cast<uint16_t>(std::abs(delta)));
//	}
//	diff.height = lhs.height;
//	diff.width = lhs.width;
//	return diff;
//}

//const bool frameset::isUniform(const Frameset& fs)
//{
//	return size(fs.depth) == size(fs.intensity) && size(fs.intensity) == size(fs.state);
//}

//const bool frameset::isEmpty(const Frameset& fs)
//{
//	return isEmpty(fs.depth) || isEmpty(fs.intensity) || isEmpty(fs.state);
//}
//
//const bool frameset::isValid(const Frameset& fs)
//{
//	return isValid(fs.depth) && isValid(fs.intensity) && isValid(fs.state);
//}

//void frameset::mask(Frameset& fs, const QRectF& maskNorm)
//{
//	mask(fs.depth, maskNorm);
//	mask(fs.intensity, maskNorm);
//	mask(fs.state, maskNorm);
//}