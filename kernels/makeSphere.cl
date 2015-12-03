#include <kernel_particle.h>

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void	makeSphere(__global t_particle *p, int const radius)
{
	int			i = get_global_id(0);
	int			x, y, z, w;
	int			val;
	double		theta;
	double		phi;

	x = 1 + i;
	y = 1 + i;
	z = 1 + i;
	w = 1 + i;
	val = x ^ (x << 11);
	x = y;
	y = z;
	z = w;
	w = w ^ (w >> 19) ^ val ^ (val >> 8);
	theta = cos((double)w);
	phi = cos((double)val);
	p[i].pos[0] = radius * theta * phi;
	theta = cos((double)w);
	phi = sin((double)val);
	p[i].pos[1] = radius * theta * phi;
	theta = sin((double)w);
	p[i].pos[2] = radius * theta;
	p[i].acc[0] = 0.0;
	p[i].acc[1] = 0.0;
	p[i].acc[2] = 0.0;
	p[i].vel[0] = 0.0;
	p[i].vel[1] = 0.0;
	p[i].vel[2] = 0.0;
	p[i].life = 1.0;
}
