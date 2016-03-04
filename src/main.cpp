
#include "Core.hpp"

void
help(void)
{
	std::cerr << "usage:" << std::endl;
	std::cerr << "./particle_system [particles]" << std::endl;
	std::cerr << "Particle number default value is: " << 1024000 << std::endl;
}

int
getRealParticleNumber(int pn, int const &wgsize)
{
	int const upper = 3000 * wgsize;
	pn = pn < wgsize ? wgsize : pn;
	pn = pn > upper ? upper : pn;
	int const rpn = (pn / wgsize) * wgsize;
	if (rpn != pn)
	{
		std::cerr << "Number of particles converted to " << rpn << " to fit kernel workgroup size." << std::endl;
	}
	return (rpn);
}

int			main(int c, char **v)
{
	Core	core;
	int		pn;

	if (c != 2)
	{
		help();
		pn = 1024000;
	}
	else
		pn = getRealParticleNumber(std::atoi(v[1]), 1024);
	if (!core.init(pn))
		return (0);
	core.loop();
	return (1);
}
