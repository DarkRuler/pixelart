#include "pixelart_thread.h"
#include <Windows.h>

cv::Mat pixelart_thread::run(uint16_t cpu_num, color_set* _color_set, cv::Mat& image)
{
	SetThreadAffinityMask(GetCurrentThread(), 1 << cpu_num);

	std::unique_ptr<pixelart> _pixelart = std::make_unique<pixelart>();
	_pixelart->set_color_set(_color_set);
	_pixelart->create(image);
	return _pixelart->get_pixelart_image();
}

