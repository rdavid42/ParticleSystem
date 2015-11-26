
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

# include <GLFW/glfw3.h>
# include "Mat4.hpp"
# include "Mat4Stack.hpp"
# include "Utils.hpp"

# define PARTICLE_NUMBER		1000000

typedef struct		s_particle
{
	float			pos[3];
	float			vel[3];
	float			acc[3];
}					t_particle;

class Core
{
public:
	t_particle				*particles;

	/* glfw */
	GLFWwindow				*window;
	int						windowWidth;
	int						windowHeight;

	/* shaders */
	GLuint					particleShader;
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

	Core(void);
	~Core(void);

	/* core */
	int						init(void);
	void					update(void);
	void					render(void);
	void					loop(void);
	
	/* particles */
	void					resetParticles(void);
	void					initParticle(t_particle *p, float &x, float &y, float &z);
	int						createSphere(void);

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
	void					attachShaders(void);
	int						linkProgram(void);
	void					deleteShaders(void);
	int						initShaders(void);

	Core &					operator=(Core const &rhs);

private:
	Core(Core const &src);
};

#endif