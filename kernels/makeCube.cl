#include <kernel_particle.h>

__kernel void makeCube(__global t_particle *p, int const m,  int const num)
{
	int		i = get_global_id(0);
	int x, y, z, w;
	int		val;
	x = 1 + i;
	y = 1 + i;
	z = 1 + i;
	w = 1 + i;
	val = x ^ (x << 11);
    x = y;
	y = z;
	z = w;
    w = w ^ (w >> 19) ^ val ^ (val >> 8);
	p[i].pos[0] = (w % (2 * m) - m);
	val = x ^ (x << 11);
    x = y;
	y = z;
	z = w;
    w = w ^ (w >> 19) ^ val ^ (val >> 8);
	p[i].pos[1] = (w % (2 * m) - m);
	val = x ^ (x << 11);
    x = y;
	y = z;
	z = w;
    w = w ^ (w >> 19) ^ val ^ (val >> 8);
	p[i].pos[2] = (w % (2 * m) - m);
	p[i].acc[0] = 0.0;
	p[i].acc[1] = 0.0;
	p[i].acc[2] = 0.0;
	p[i].vel[0] = 0.0;
	p[i].vel[1] = 0.0;
	p[i].vel[2] = 0.0;
}
