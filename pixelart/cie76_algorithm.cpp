#include "cie76_algorithm.h"
#include <opencv2\imgproc\imgproc.hpp>
#include <cmath>

double cie76_algorithm::diff_algorithm(color source_pixel, color sample_pixel)
{
	return sqrt(pow(source_pixel[0] - sample_pixel[0], 2) + pow(source_pixel[1] - sample_pixel[1], 2) + pow(source_pixel[2] - sample_pixel[2], 2));
}

color cie76_algorithm::rgb_to_lab(color pixel_rgb)
{
	color pixel_lab = color(3);
	cv::cvtColor(pixel_rgb, pixel_lab, CV_RGB2Lab);
	return pixel_lab;
}
