#include <kernel_particle.h>

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void	makeTorus(__global t_particle *p, int const radius)
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
	theta = native_cos((double)w);
	phi = native_cos((double)val);
	p[i].pos[0] = (60 + 30 * theta) * phi;
 	theta = native_cos((double)w);
 	phi = native_sin((double)val);
	p[i].pos[1] = (60 + 30 * theta) * phi;
 	phi = native_sin((double)val);
	p[i].pos[2] = 30 * phi;
	p[i].acc[0] = 0.0;
	p[i].acc[1] = 0.0;
	p[i].acc[2] = 0.0;
	p[i].vel[0] = 0.0;
	p[i].vel[1] = 0.0;
	p[i].vel[2] = 0.0;
	p[i].life = 1.0;
}
