
#ifndef CORE_HPP
# define CORE_HPP

# include "Utils.hpp"
# include "Camera.hpp"

# define		ACCELERATION_KERNEL		0
# define		UPDATE_KERNEL			1
# define		MAKECUBE_KERNEL			2
# define		MAKESPHERE_KERNEL		3
# define		RESET_KERNEL			4
# define		SPRAY_EMITTER_KERNEL	5
# define		MAKETORUS_KERNEL		6

# define		N_PROGRAM				7
# define		PARTICLE_NUMBER			(1024000 * 3)

typedef struct		s_particle
{
	float			pos[3];
	float			vel[3];
	float			acc[3];
	float			life;
}					t_particle;

typedef struct		s_emitter
{
	float			pos[3];
}					t_emitter;

class Core
{
public:
	cl_mem					dp; // device particles

	// camera particle plane
	float					particlePlaneHeight;
	float					particlePlaneWidth;

	// OpenCL
	size_t					local[N_PROGRAM];
	size_t					global;
	size_t					emitter_global;
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

	/* Locations */
	GLuint					projLoc;
	GLuint					viewLoc;
	GLuint					objLoc;
	GLuint					positionLoc;
	GLuint					colorLoc;
	GLuint					redLoc;
	GLuint					greenLoc;
	GLuint					blueLoc;
	GLuint					rredLoc;
	GLuint					rgreenLoc;
	GLuint					rblueLoc;
	GLuint					lifeLoc;
	GLuint					emitterActiveLoc;
	GLuint					pVao;
	GLuint					pVbo;

	Vec3<float>				magnet;
	Vec3<float>				gravityPos;
	bool					gravity;
	bool					line;
	double					particleSize;
	double					particleSizeInc;
	double					particleSizeMin;
	double					particleSizeMax;

	bool					emitterActive;

	/* Camera */
	Camera					camera;
	bool					cameraActive;

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

	/* textures */
	GLuint					loadTexture(char const *filename);

	/* magnet */
	void					moveMagnet(double xpos, double ypos);


	/* matrices */ 
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
	cl_int					getOpenCLInfo(void);
	cl_int					launchKernelsResetShape(int kernel);
	cl_int					launchKernelSprayEmitter(void);
	cl_int					launchKernelsAcceleration(int const &state, Vec3<float> const &pos);
	cl_int					launchKernelsUpdate(void);
	cl_int					launchKernelReset(void);
	cl_int					initParticles();
	cl_int					cleanDeviceMemory(void);

	Core &					operator=(Core const &rhs);

private:
	Core(Core const &src);
};

#endif
