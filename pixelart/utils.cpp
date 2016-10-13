#include "utils.h"
#include <cmath>

std::vector<std::string> utils::split_string(std::string input, std::string delimiter)
{
	std::vector<std::string> tokens = std::vector<std::string>();

	size_t pos = 0;
	std::string token;

	while ((pos = input.find(delimiter)) != std::string::npos) {
		token = input.substr(0, pos);
		tokens.push_back(token);
		input.erase(0, pos + delimiter.length());
	}

	tokens.push_back(input);

	return tokens;
}

std::vector<unsigned int> utils::calculate_thresholds(const unsigned int threads, const unsigned int height)
{
	std::vector<unsigned int> thresholds = std::vector<unsigned int>();
	for (unsigned int thread_index = 0; thread_index < threads+1; thread_index++)
	{
		thresholds.push_back((unsigned int)std::round(height*thread_index/threads));
	}
	return thresholds;
}
