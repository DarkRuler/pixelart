#pragma once
#include <cstring>
#include <unordered_map>
#include <cinttypes>
#include <functional>
#include <tuple>

typedef std::vector<uint16_t> color;

class color_set
{
private:
	std::unordered_map<std::string, color>* colors;
	std::function<double(color, color)> diff_algorithm;

public:
	color_set();
	~color_set();

	void add_modify_color(std::string color_name, color _color);
	void remove_color(std::string color_name);
	
	void replace_color_set(std::unordered_map<std::string, color>* _colors);
	void set_diff_algorithm(std::function<double(color, color)> diff_algorithm);

	std::unordered_map<std::string, color>* get_colors();
	std::tuple<std::string, color> evaluate_closest_color(color evaluated_color);
};

