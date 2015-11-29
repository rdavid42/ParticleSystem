
#include "Core.hpp"

int			main(void)
{
	Core	core;

	if (!core.init())
		return (0);
	std::cerr << "4" << std::endl;
	core.loop();
	return (1);
}
