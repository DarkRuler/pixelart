#pragma once
#include <vector>
typedef std::vector<uint16_t> color;

class cie76_algorithm
{
public:
	static double diff_algorithm(color source_pixel, color sample_pixel);
	static color rgb_to_lab(color pixel_rgb);
};

