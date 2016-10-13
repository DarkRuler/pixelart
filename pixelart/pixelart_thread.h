#pragma once
#include "pixelart.h"
#include <stddef.h>
#include <opencv2/highgui/highgui.hpp>
#include <memory>

class pixelart_thread
{
public:
	static cv::Mat run(uint16_t cpu_num, color_set* _color_set, cv::Mat& image);
};

