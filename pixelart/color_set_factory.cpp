#include "color_set_factory.h"
#include <fstream>
#include <sstream>
#include <json.hpp>
#include <vector>
#include "utils.h"

color_set * color_set_factory::create_mono_scheme(color _color)
{
	color_set* mono_scheme = new color_set();
	mono_scheme->add_modify_color("...", _color);
	return mono_scheme;
}

color_set * color_set_factory::create_wool_scheme()
{
	color_set* wool_scheme = new color_set();

	wool_scheme->add_modify_color("white", { 207, 175, 158 });
	wool_scheme->add_modify_color("orange", { 159, 81, 35 });
	wool_scheme->add_modify_color("magenta", { 147, 85, 105 });
	wool_scheme->add_modify_color("light_blue", { 110, 105, 135 });
	wool_scheme->add_modify_color("yellow", { 183, 130, 32 });
	wool_scheme->add_modify_color("lime", { 100, 114, 50 });
	wool_scheme->add_modify_color("pink", { 159, 75, 76 });
	wool_scheme->add_modify_color("gray", { 55, 39, 32 });
	wool_scheme->add_modify_color("light_gray", { 132, 104, 94 });
	wool_scheme->add_modify_color("cyan", { 84, 88, 88 });
	wool_scheme->add_modify_color("purple", { 115, 67, 83 });
	wool_scheme->add_modify_color("blue", { 71, 57, 88 });
	wool_scheme->add_modify_color("brown", { 74, 48, 32 });
	wool_scheme->add_modify_color("green", { 73, 80, 39 });
	wool_scheme->add_modify_color("red", { 140, 58, 44 });
	wool_scheme->add_modify_color("black", { 34, 19, 13 });

	return wool_scheme;
}

color_set * color_set_factory::create_hardened_clay_scheme()
{
	color_set* hardened_clay_scheme = new color_set();

	hardened_clay_scheme->add_modify_color("white", { 221, 221, 221 });
	hardened_clay_scheme->add_modify_color("orange", { 219, 125, 62 });
	hardened_clay_scheme->add_modify_color("magenta", { 179, 80, 188 });
	hardened_clay_scheme->add_modify_color("light_blue", { 107, 138, 201 });
	hardened_clay_scheme->add_modify_color("yellow", { 177, 166, 39 });
	hardened_clay_scheme->add_modify_color("lime", { 65, 174, 56 });
	hardened_clay_scheme->add_modify_color("pink", { 208, 132, 153 });
	hardened_clay_scheme->add_modify_color("gray", { 64, 64, 64 });
	hardened_clay_scheme->add_modify_color("light_gray", { 154, 161, 161 });
	hardened_clay_scheme->add_modify_color("cyan", { 46, 110, 137 });
	hardened_clay_scheme->add_modify_color("purple", { 126, 61, 181 });
	hardened_clay_scheme->add_modify_color("blue", { 46, 56, 141 });
	hardened_clay_scheme->add_modify_color("brown", { 79, 50, 31 });
	hardened_clay_scheme->add_modify_color("green", { 53, 70, 27 });
	hardened_clay_scheme->add_modify_color("red", { 150, 52, 48 });
	hardened_clay_scheme->add_modify_color("black", { 25, 22, 22 });

	return hardened_clay_scheme;
}

color_set * color_set_factory::create_generic_blocks_scheme()
{
	return nullptr;
}

color_set * color_set_factory::parse_json_file(const char * filepath)
{
	std::fstream color_scheme_file = std::fstream(filepath, std::ios::in);
	if (color_scheme_file.bad()) return nullptr;

	std::stringstream cs_file_buffer;
	cs_file_buffer << color_scheme_file.rdbuf();

	json::JSON cs_file_json = json::JSON::Load(cs_file_buffer.str());

	color_set* file_cs = new color_set();

	auto colors_range = cs_file_json.ObjectRange();
	for (auto color : colors_range)
	{
		std::vector<std::string> rgb_string = utils::split_string(color.second.ToString(), ",");		
		file_cs->add_modify_color(color.first, { (unsigned short)std::stoi(rgb_string[0]), (unsigned short)std::stoi(rgb_string[1]), (unsigned short)std::stoi(rgb_string[2]) });
	}

	return file_cs;
}

color_set * color_set_factory::merge(std::vector<color_set*> sets)
{
	color_set* merged = new color_set();

	int set_index = 0;
	for (color_set* _color_set : sets)
	{
		std::unordered_map<std::string, color>* colors = _color_set->get_colors();
		for (auto color_iter : *colors)
		{
			std::string prefix = set_index + " ";
			merged->add_modify_color(prefix + color_iter.first, color_iter.second);
		}
		set_index++;
	}

	return merged;
}
