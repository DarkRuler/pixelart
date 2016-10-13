#pragma once
#include <opencv2/highgui/highgui.hpp>

class color_set;

class application
{
private:
	
public:
	application();
	application(int argc, char** argv);
	~application();

	cv::Mat create_pixelart_multithread(cv::Mat& source_image, color_set* _color_set, unsigned int cpu_num, unsigned int thread_num);
	int run();
};