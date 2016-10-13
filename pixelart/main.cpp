#include "application.h"
#include <memory>

int main(int argc, char** argv)
{
	std::unique_ptr<application> _main = std::make_unique<application>(argc, argv);
	return _main->run();
}