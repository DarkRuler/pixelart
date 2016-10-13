#include "color_set.h"

color_set::color_set()
{
	colors = new std::unordered_map<std::string, color>();
}

color_set::~color_set()
{
	delete colors;
}

void color_set::add_modify_color(std::string color_name, color _color)
{
	(*colors)[color_name] = _color;
}

void color_set::remove_color(std::string color_name)
{
	colors->erase(color_name);
}

void color_set::replace_color_set(std::unordered_map<std::string, color>* colors)
{
	this->colors = colors;
}

void color_set::set_diff_algorithm(std::function<double(color, color)> diff_algorithm)
{
	this->diff_algorithm = diff_algorithm;
}

std::unordered_map<std::string, color>* color_set::get_colors()
{
	return colors;
}

std::tuple<std::string, color> color_set::evaluate_closest_color(color evaluated_color)
{
	double minimal_value = std::numeric_limits<double>::infinity();
	std::tuple<std::string, color> closest_color;

	for (auto color_iterator : *colors)
	{
		double color_diff_value = diff_algorithm(evaluated_color, color_iterator.second);
		if (color_diff_value < minimal_value)
		{
			minimal_value = color_diff_value;
			closest_color = std::make_tuple(color_iterator.first, color_iterator.second);
		}
	}

	return closest_color;
}
