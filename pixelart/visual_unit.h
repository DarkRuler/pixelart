#pragma once
#include <opencv2\imgproc\imgproc.hpp>

class visual_unit
{
public:
	static cv::Mat load_image(const std::string& filename);
	static cv::Mat resize(const cv::Mat& input_image, size_t width, size_t height);
	static void show(const char* title, cv::Mat& image);
};