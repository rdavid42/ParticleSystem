
#include <kernel_particle.h>
#include <acceleration.cl>

__kernel void sprayEmitter(__global t_particle *p, float3 const emitter, float3 const pointer)
{
	float		h;
	int const	i = get_global_id(0);
	float		tmp = 50.0;
	int			seed;
	int			val;
	int			w;

	if (p[i].life <= 0.0)
	{
		seed = i + 1;
		w = i + 1;
		val = seed ^ (seed << 11);
		w = w ^ (w >> 19) ^ val ^ (val >> 8);
		p[i].life = 1.0;
		p[i].pos[0] = emitter[0];
		p[i].pos[1] = emitter[1];
		p[i].pos[2] = emitter[2];
		p[i].vel[0] = 0.0;
		p[i].vel[1] = 0.0;
		p[i].vel[2] = 0.0;
		p[i].acc[0] = (pointer[0] - emitter[0]) + cos((float)w) * 10.0;
		p[i].acc[1] = (pointer[1] - emitter[1]) + sin((float)w) * 10.0;
		p[i].acc[2] = (pointer[2] - emitter[2]) + tan((float)w) * 10.0;
		h = sqrt(p[i].acc[0] * p[i].acc[0] + p[i].acc[1] * p[i].acc[1] + p[i].acc[2] * p[i].acc[2]) * tmp;
		p[i].acc[0] /= h;
		p[i].acc[1] /= h;
		p[i].acc[2] /= h;
	}
	p[i].life -= DECAY;
	update(p);
}
