#pragma once
#include "color_set.h"

class color_set_factory
{
public:
	static color_set* create_mono_scheme(color _color);

	static color_set* create_wool_scheme();
	static color_set* create_hardened_clay_scheme();	
	static color_set* create_generic_blocks_scheme();
		
	static color_set* parse_json_file(const char* filepath);

	static color_set* merge(std::vector<color_set*> sets);
};

