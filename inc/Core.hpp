
#ifndef CORE_HPP
# define CORE_HPP

# if defined(__APPLE_CC__)
#  ifndef GLFW_INCLUDE_GLCOREARB
#   define GLFW_INCLUDE_GLCOREARB
#  endif
#  ifndef GLFW_INCLUDE_GLEXT
#   define GLFW_INCLUDE_GLEXT
#  endif
# else
#  define GL_GLEXT_PROTOTYPES
# endif

# ifdef __APPLE__
#  include <OpenCL/opencl.h>
#  include <OpenGL/CGLTypes.h>
#  include <OpenGL/CGLCurrent.h>
# else
#  define GL_GLEXT_PROTOTYPES
#  include <CL/cl.h>
# endif

# include <GLFW/glfw3.h>
# include "Mat4.hpp"
# include "Mat4Stack.hpp"
# include "Utils.hpp"

# define		ACCELERATION_KERNEL		0
# define		UPDATE_KERNEL			1

# define		N_PROGRAM				2
# define		PARTICLE_NUMBER			1024000

typedef struct		s_particle
{
	float			pos[3];
	float			vel[3];
	float			acc[3];
}					t_particle;

class Core
{
public:
	cl_mem					dp; // device particles

	// OpenCL
	size_t					local[N_PROGRAM];
	size_t					global;
	cl_uint					num_entries;
	cl_platform_id			platformID;
	cl_uint					num_platforms;
	cl_device_id			clDeviceId;
	cl_context				clContext;
	cl_command_queue		clCommands;
	cl_program				clPrograms[N_PROGRAM];
	cl_kernel				clKernels[N_PROGRAM];
	std::ostringstream		oss_ticks;

	/* glfw */
	GLFWwindow				*window;
	int						windowWidth;
	int						windowHeight;

	/* shaders */
	GLuint					vertexShader;
	GLuint					fragmentShader;
	GLuint					program;

	/* matrices */
	Mat4Stack<float>		ms;
	Mat4<float>				projMatrix;
	Mat4<float>				viewMatrix;

	/* camera */
	Vec3<float>				cameraPos;
	Vec3<float>				cameraLookAt;

	/* Locations */
	GLuint					projLoc;
	GLuint					viewLoc;
	GLuint					objLoc;
	GLuint					positionLoc;
	GLuint					colorLoc;

	GLuint					pVao;
	GLuint					pVbo;

	Vec3<float>				magnet;

	Core(void);
	~Core(void);

	/* core */
	int						init(void);
	void					magnetInit(void);
	void					update(void);
	void					render(void);
	void					loop(void);
	
	/* cpu temporary particles */
	void					resetParticles(t_particle *hp);
	void					initParticle(t_particle *p, float &x, float &y, float &z);
	int						createSphere(t_particle *hp);

	/* matrices */ 
	void					setViewMatrix(Mat4<float> &view, Vec3<float> const &dir,
										Vec3<float> const &right, Vec3<float> const &up);
	void					setCamera(Mat4<float> &view, Vec3<float> const &pos, Vec3<float> const &lookAt);
	void					buildProjectionMatrix(Mat4<float> &proj, float const &fov,
												float const &near, float const &far);

	/* shaders */
	void					getLocations(void);
	int						compileShader(GLuint shader, char const *filename);
	GLuint					loadShader(GLenum type, char const *filename);
	int						loadShaders(void);
	int						linkProgram(GLuint &p);
	void					deleteShaders(void);
	int						initShaders(void);
	/* OpenCL */
	cl_int					initOpencl(void);
	cl_int					launchKernelsAcceleration(int const &state, Vec3<float> const &pos);
	cl_int					launchKernelsUpdate(void);
	cl_int					initParticles(void);
	cl_int					cleanDeviceMemory(void);
/*	cl_int					writeDeviceParticles(void);*/

	Core &					operator=(Core const &rhs);

private:
	Core(Core const &src);
};

#endif