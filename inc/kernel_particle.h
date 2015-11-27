#ifndef KERNEL_PARTICLE
# define KERNEL_PARTICLE

# define ACC		(6.0)

typedef struct		s_particle
{
	float			pos[3];
	float			vel[3];
	float			acc[3];
}					t_particle;

#endif