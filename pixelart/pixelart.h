#pragma once
#include <opencv2/core/core.hpp>
#include "color_set.h"

class pixelart
{
private:
	color_set* colors;
	cv::Mat pixelart_image;

public:
	pixelart();
	~pixelart();

	void set_color_set(color_set* new_colors);
	
	void create(cv::Mat source_image);
	cv::Mat get_pixelart_image();
};

