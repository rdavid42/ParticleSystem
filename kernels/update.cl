
#include <kernel_particle.h>

__kernel void update(__global t_particle *p)
{
	int const	i = get_global_id(0);

	p[i].vel[0] += p[i].acc[0];
	p[i].vel[1] += p[i].acc[1];
	p[i].vel[2] += p[i].acc[2];
	p[i].pos[0] += p[i].vel[0];
	p[i].pos[1] += p[i].vel[1];
	p[i].pos[2] += p[i].vel[2];
	if (p[i].vel[0] < -3)
		p[i].vel[0] = -3;
	else if (p[i].vel[0] > 3)
		p[i].vel[0] = 3;
	if (p[i].vel[1] < -3)
		p[i].vel[1] = -3;
	else if (p[i].vel[1] > 3)
		p[i].vel[1] = 3;
	if (p[i].vel[2] < -3)
		p[i].vel[2] = -3;
	else if (p[i].vel[2] > 3)
		p[i].vel[2] = 3;
}
