#include <kernel_particle.h>

__kernel void reset(__global t_particle *p)
{
	int		i = get_global_id(0);

	p[i].pos[0] = 0.0;
	p[i].pos[1] = 0.0;
	p[i].pos[2] = 0.0;
	p[i].acc[0] = 0.0;
	p[i].acc[1] = 0.0;
	p[i].acc[2] = 0.0;
	p[i].vel[0] = 0.0;
	p[i].vel[1] = 0.0;
	p[i].vel[2] = 0.0;
	p[i].life = 0.0;
}
