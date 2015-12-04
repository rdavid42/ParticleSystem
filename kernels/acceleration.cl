
#include <kernel_particle.h>
#include <update.cl>

__kernel void acceleration(__global t_particle *p, int const state, float3 const c)
{
	int const		i = get_global_id(0);
	float			h;
	float const		tmp = state;//ACC / state;

	p[i].acc[0] = c[0] - p[i].pos[0];
	p[i].acc[1] = c[1] - p[i].pos[1];
	p[i].acc[2] = c[2] - p[i].pos[2];
	h = sqrt(p[i].acc[0] * p[i].acc[0] + p[i].acc[1] * p[i].acc[1] + p[i].acc[2] * p[i].acc[2]) * tmp;
	p[i].acc[0] /= h;
	p[i].acc[1] /= h;
	p[i].acc[2] /= h;
	// UPDATE
	update(p);
}
