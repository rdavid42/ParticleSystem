
#include <kernel_particle.h>

__kernel void acceleration(__global t_particle *p, int const state, float const cx, float const cy, float const cz)
{
	int const		i = get_global_id(0);
	float			h;
	float const		tmp = state;

	p[i].acc[0] = cx - p[i].pos[0];
	p[i].acc[1] = cy - p[i].pos[1];
	p[i].acc[2] = cz - p[i].pos[2];
	h = sqrt(p[i].acc[0] * p[i].acc[0] + p[i].acc[1] * p[i].acc[1] + p[i].acc[2] * p[i].acc[2]);
	p[i].acc[0] /= h;
	p[i].acc[1] /= h;
	p[i].acc[2] /= h;
	p[i].acc[0] /= tmp;
	p[i].acc[1] /= tmp;
	p[i].acc[2] /= tmp;
	// UPDATE
	
	p[i].vel[0] += p[i].acc[0];
	p[i].vel[1] += p[i].acc[1];
	p[i].vel[2] += p[i].acc[2];
	p[i].pos[0] += p[i].vel[0];
	p[i].pos[1] += p[i].vel[1];
	p[i].pos[2] += p[i].vel[2];

// 	if (p[i].vel[0] < -VEL_CAP)
// 		p[i].vel[0] = -VEL_CAP;
// 	if (p[i].vel[0] > VEL_CAP)
// 		p[i].vel[0] = VEL_CAP;
// 	if (p[i].vel[1] < -VEL_CAP)
// 		p[i].vel[1] = -VEL_CAP;
// 	if (p[i].vel[1] > VEL_CAP)
// 		p[i].vel[1] = VEL_CAP;
// 	if (p[i].vel[2] < -VEL_CAP)
// 		p[i].vel[2] = -VEL_CAP;
// 	if (p[i].vel[2] > VEL_CAP)
// 		p[i].vel[2] = VEL_CAP;
}
