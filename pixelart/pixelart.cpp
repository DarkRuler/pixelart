#include "pixelart.h"

pixelart::pixelart()
{
	colors = new color_set();
}

void pixelart::set_color_set(color_set* new_colors)
{
	colors = new_colors;
}

void pixelart::create(cv::Mat source_image)
{
	pixelart_image = cv::Mat(source_image.rows, source_image.cols, source_image.type());

	int channels = source_image.channels();
	for (size_t x_index = 0; x_index < source_image.rows; x_index++)
	{
		uchar* source_pointer = source_image.ptr<uchar>(x_index);
		uchar* pixelart_pointer = pixelart_image.ptr<uchar>(x_index);
		
		for (size_t y_index = 0; y_index < source_image.cols; y_index++)
		{
			color source_pixel = { source_pointer[y_index*channels + 2], source_pointer[y_index*channels + 1], source_pointer[y_index*channels + 0] };
			color pixelart_pixel = std::get<1>(colors->evaluate_closest_color(source_pixel));

			pixelart_pointer[y_index*channels + 0] = pixelart_pixel[2];
			pixelart_pointer[y_index*channels + 1] = pixelart_pixel[1];
			pixelart_pointer[y_index*channels + 2] = pixelart_pixel[0];
		}
	}
}

cv::Mat pixelart::get_pixelart_image()
{
	return pixelart_image;
}

pixelart::~pixelart()
{

}