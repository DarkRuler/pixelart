#pragma once
#include <vector>
#include <string>

class utils
{
public:
	static std::vector<std::string> split_string(std::string input, std::string delimiter);
	static std::vector<unsigned int> calculate_thresholds(const unsigned int threads, const unsigned int height);
};

