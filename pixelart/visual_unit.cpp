#include "visual_unit.h"
#include <opencv2\highgui\highgui.hpp>

cv::Mat visual_unit::load_image(const std::string & filename)
{
	return cv::imread(filename, cv::IMREAD_COLOR);
}

cv::Mat visual_unit::resize(const cv::Mat & input_image, size_t width, size_t height)
{
	cv::Mat resized_image = cv::Mat(height, width, CV_8UC3);
	cv::resize(input_image, resized_image, resized_image.size());
	return resized_image;
}

void visual_unit::show(const char* title, cv::Mat& image)
{
	cv::namedWindow(std::string(title), CV_WINDOW_KEEPRATIO);
	cv::imshow(std::string(title), image);
}