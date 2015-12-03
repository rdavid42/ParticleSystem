#ifndef KERNEL_PARTICLE
# define KERNEL_PARTICLE

// OPENCL HEADER

# define ACC		(6.0)
# define VEL_CAP	(3.0)
# define DECAY		(0.005)

typedef struct		s_particle
{
	float			pos[3];
	float			vel[3];
	float			acc[3];
	float			life;
}					t_particle;

#endif
