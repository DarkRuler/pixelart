#include "application.h"
#include "visual_unit.h"
#include "pixelart.h"
#include "color_set_factory.h"
#include "cie76_algorithm.h"
#include "pixelart_thread.h"
#include "utils.h"

#include <iostream>
#include <future>
#include <thread>

application::application()
{
	
}

application::application(int argc, char ** argv)
{
}

application::~application()
{
}

double diff_algorithm(color source_color, color sample_color)
{
	return 1;
}

cv::Mat application::create_pixelart_multithread(cv::Mat& source_image, color_set* _color_set, unsigned int cpu_num, unsigned int thread_num)
{
	unsigned int max_cpu_num = std::thread::hardware_concurrency();
	const unsigned int actual_cpu_num = max_cpu_num > cpu_num ? cpu_num : max_cpu_num;

	cv::Mat source_copy = source_image.clone();
	std::vector<cv::Mat> source_parts = std::vector<cv::Mat>();
	std::vector<unsigned int> thresholds = utils::calculate_thresholds(thread_num, source_image.rows);

	for (unsigned int part_index = 0; part_index < thread_num; part_index++)
	{
		source_parts.push_back(source_copy(cv::Rect(0, thresholds[part_index], source_image.cols, thresholds[part_index + 1] - thresholds[part_index])));
	}

	std::vector<std::future<cv::Mat>> future_values_set = std::vector<std::future<cv::Mat>>();
	for (unsigned int thread_index = 0; thread_index < thread_num; thread_index++)
	{
		future_values_set.push_back(std::async(std::launch::async, &pixelart_thread::run, (unsigned int)std::round(actual_cpu_num*thread_index/thread_num), _color_set, source_parts[thread_index]));
	}

	std::vector<cv::Mat> finalized_pixelart_parts;
	for (unsigned int thread_index = 0; thread_index < thread_num; thread_index++)
	{
		finalized_pixelart_parts.push_back(future_values_set[thread_index].get());
	}

	cv::Mat full_pixelart; 
	cv::vconcat(finalized_pixelart_parts, full_pixelart);

	return full_pixelart;
}

int application::run()
{
	std::string filepath = "H:\\visuals\\images\\album_art\\51dVh9OgULL._SL1118_.jpg";
	cv::Mat original_image = visual_unit::load_image(filepath);
	if (!original_image.data) return -1;

	const uint32_t width = 256;
	const uint32_t height = 256;

	cv::Mat pixelated_image = visual_unit::resize(original_image, width, height);
	if (!pixelated_image.data) return -1;

	std::unique_ptr<color_set> _color_set(color_set_factory::parse_json_file("C:\\Users\\Dave_Bowman\\Documents\\Visual Studio 2015\\Projects\\pixelart\\pixelart\\json\\map_colors_simple.json"));
	_color_set->set_diff_algorithm(&cie76_algorithm::diff_algorithm);

	cv::Mat pixelart = create_pixelart_multithread(pixelated_image, _color_set.get(), 4, 4);
	visual_unit::show("[merged_pixelart]", pixelart);

	visual_unit::show("[pixelated]", pixelated_image);	
	cv::waitKey();
	
	return 0;
}
