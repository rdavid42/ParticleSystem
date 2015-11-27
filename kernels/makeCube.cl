#include <kernel_particle.h>

__kernel void makeCube(__global t_particle *p, int const m,  int const num)
{
	int		i = get_global_id(0);

	p[i].acc[0] = 0.0;
	p[i].acc[1] = 0.0;
	p[i].acc[2] = 0.0;
	p[i].pos[0] = i % (2 * m) - m + (i / num) ;
	p[i].pos[1] = i % (2 * m) - m + (i / num) ;
	p[i].pos[2] = i % (2 * m) - m + (i / num) ;
}
