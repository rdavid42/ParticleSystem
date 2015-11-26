
#include "Core.hpp"

Core::Core(void)
{
}

Core::~Core(void)
{
	glfwDestroyWindow(this->window);
	glfwTerminate();
}

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	Core		*core = static_cast<Core *>(glfwGetWindowUserPointer(window));

	(void)scancode;
	(void)mods;
	(void)core;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void
Core::buildProjectionMatrix(Mat4<float> &proj, float const &fov,
							float const &near, float const &far)
{
	float const			f = 1.0f / tan(fov * (M_PI / 360.0));
	float const			ratio = (1.0f * this->windowWidth) / this->windowHeight;

	proj.setIdentity();
	proj[0] = f / ratio;
	proj[1 * 4 + 1] = f;
	proj[2 * 4 + 2] = (far + near) / (near - far);
	proj[3 * 4 + 2] = (2.0f * far * near) / (near - far);
	proj[2 * 4 + 3] = -1.0f;
	proj[3 * 4 + 3] = 0.0f;
}

void
Core::setViewMatrix(Mat4<float> &view, Vec3<float> const &dir,
					Vec3<float> const &right, Vec3<float> const &up)
{
	/*
	rx		ux		-dx		0
	ry		uy		-dy		0
	rz		uz		-dz		0
	0		0		0		1
	*/
	// first column
	view[0] = right.x;
	view[4] = right.y;
	view[8] = right.z;
	view[12] = 0.0f;
	// second column
	view[1] = up.x;
	view[5] = up.y;
	view[9] = up.z;
	view[13] = 0.0f;
	// third column
	view[2] = -dir.x;
	view[6] = -dir.y;
	view[10] = -dir.z;
	view[14] = 0.0f;
	// fourth column
	view[3] = 0.0f;
	view[7] = 0.0f;
	view[11] = 0.0f;
	view[15] = 1.0f;
}

void
Core::setCamera(Mat4<float> &view, Vec3<float> const &pos, Vec3<float> const &lookAt)
{
	Vec3<float>		dir;
	Vec3<float>		right;
	Vec3<float>		up;
	Mat4<float>		translation;

	up.set(0.0f, 1.0f, 0.0f);
	dir.set(lookAt - pos);
	dir.normalize();
	right.crossProduct(dir, up);
	right.normalize();
	up.crossProduct(right, dir);
	up.normalize();
	this->setViewMatrix(view, dir, right, up);
	translation.setTranslation(-pos.x, -pos.y, -pos.z);
	view.multiply(translation);
}

void
Core::resetParticles(void)
{
	int				i;

	for (i = 0; i < PARTICLE_NUMBER; ++i)
	{
		this->particles[i].pos[0] = 0;
		this->particles[i].pos[1] = 0;
		this->particles[i].pos[2] = 0;
		this->particles[i].vel[0] = 0;
		this->particles[i].vel[1] = 0;
		this->particles[i].vel[2] = 0;
		this->particles[i].acc[0] = 0;
		this->particles[i].acc[1] = 0;
		this->particles[i].acc[2] = 0;
	}
}

void
Core::initParticle(t_particle *p, float &x, float &y, float &z)
{
	p->pos[0] = x;
	p->pos[1] = y;
	p->pos[2] = z;
	p->vel[0] = 0;
	p->vel[1] = 0;
	p->vel[2] = 0;
	p->acc[0] = 0;
	p->acc[1] = 0;
	p->acc[2] = 0;
}

int
Core::createSphere(void)
{
	float					x;
	float					y;
	float					z;
	// Vec3<float>				camPos(0.0f, 0.0f, 90.0f);
	int						i;
	int						j;
	static int const		m = std::cbrt(PARTICLE_NUMBER) / 2;
	static double const		inc = 0.8;

	i = 0;
	j = 0;
	y = -m;
	while (y < m)
	{
		z = -m;
		while (z < m)
		{
			x = -m;
			while (x < m)
			{
				if (i < PARTICLE_NUMBER)
				{
					if (x * x + y * y + z * z <= m * m)
					{
						this->initParticle(&this->particles[i], x, y, z);
						++i;
					}
				}
				else
					break ;
				x += inc;
			}
			z += inc;
		}
		y += inc;
	}
	// this->camera->setPosition(camPos);
	return (1);
}

void
Core::getLocations(void)
{
	this->positionLoc = glGetAttribLocation(this->program, "position");
	this->projLoc = glGetUniformLocation(this->program, "proj_matrix");
	this->viewLoc = glGetUniformLocation(this->program, "view_matrix");
	this->colorLoc = glGetUniformLocation(this->program, "frag_color");
	this->objLoc = glGetUniformLocation(this->program, "obj_matrix");
}

int
Core::init(void)
{

	this->windowWidth = 1280;
	this->windowHeight = 1280;
	if (!glfwInit())
		return (0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	this->window = glfwCreateWindow(this->windowWidth, this->windowHeight,
									"Particle System", NULL, NULL);
	if (!this->window)
	{
		glfwTerminate();
		return (0);
	}
	glfwSetWindowUserPointer(this->window, this);
	glfwMakeContextCurrent(this->window); // make the opengl context of the window current on the main thread
	glfwSwapInterval(1); // VSYNC 60 fps max
	glfwSetKeyCallback(this->window, key_callback);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	this->buildProjectionMatrix(this->projMatrix, 53.13f, 0.1f, 1000.0f);
	this->cameraPos.set(0.5f, 0.3f, 1.5f);
	// this->cameraPos.set(5.5f, 5.5f, 5.5f);
	this->cameraLookAt.set(0.5f, 0.4f, 0.5f);
	this->setCamera(this->viewMatrix, this->cameraPos, this->cameraLookAt);
	if (!this->initShaders())
		return (0);
	this->getLocations();
	checkGlError(__FILE__, __LINE__);
	this->particles = new t_particle[PARTICLE_NUMBER];
	std::cerr << glGetString(GL_VERSION) << std::endl;
	this->resetParticles();
	/*glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(t_particle), this->particles); //float position[3]
	checkGlError(__FILE__, __LINE__);
	glEnableVertexAttribArray(0);
	checkGlError(__FILE__, __LINE__);*/

	glGenVertexArrays(1, &pVao);
	glBindVertexArray(pVao);
	glGenBuffers(1, &pVbo);
	glBindBuffer(GL_ARRAY_BUFFER, pVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(t_particle) * PARTICLE_NUMBER, this->particles, GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(t_particle), (void *)0);
	return (1);
}

int
Core::compileShader(GLuint shader, char const *filename)
{
	GLint			logsize;
	GLint			state;
	char			*compileLog;

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &state);
	if (state != GL_TRUE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
		compileLog = new char[logsize + 1];
		std::memset(compileLog, '\0', logsize + 1);
		glGetShaderInfoLog(shader, logsize, &logsize, compileLog);
		std::cerr	<< "Failed to compile shader `"
					<< filename
					<< "`: " << std::endl
					<< compileLog;
		delete compileLog;
		return (0);
	}
	return (1);
}

GLuint
Core::loadShader(GLenum type, char const *filename)
{
	GLuint			shader;
	char			*source;

	shader = glCreateShader(type);
	if (shader == 0)
		return (printError("Failed to create shader !", 0));
	if (!(source = readFile(filename)))
		return (printError("Failed to read file !", 0));
	glShaderSource(shader, 1, (char const **)&source, 0);
	if (!compileShader(shader, filename))
		return (0);
	delete source;
	return (shader);
}

int
Core::loadShaders(void)
{
	if (!(this->vertexShader = this->loadShader(GL_VERTEX_SHADER, "./shaders/vertex_shader.gls")))
		return (printError("Failed to load vertex shader !", 0));
	if (!(this->fragmentShader = this->loadShader(GL_FRAGMENT_SHADER, "./shaders/fragment_shader.gls")))
		return (printError("Failed to load fragment shader !", 0));
	return (1);
}

void
Core::attachShaders(void)
{
	glAttachShader(this->program, this->vertexShader);
	glAttachShader(this->program, this->fragmentShader);
}

int
Core::linkProgram(void)
{
	GLint			logSize;
	GLint			state;
	char			*linkLog;

	glLinkProgram(this->program);
	glGetProgramiv(this->program, GL_LINK_STATUS, &state);
	if (state != GL_TRUE)
	{
		glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &logSize);
		linkLog = new char[logSize + 1];
		std::memset(linkLog, '\0', logSize + 1);
		glGetProgramInfoLog(this->program, logSize, &logSize, linkLog);
		std::cerr	<< "Failed to link program !" << std::endl
					<< linkLog;
		delete [] linkLog;
		return (0);
	}
	return (1);
}

void
Core::deleteShaders(void)
{
	glDeleteShader(this->vertexShader);
	glDeleteShader(this->fragmentShader);
}

int
Core::initShaders(void)
{
	if (!loadShaders())
		return (0);
	if (!(this->program = glCreateProgram()))
		return (printError("Failed to create program !", 0));
	this->attachShaders();
	glBindFragDataLocation(this->program, 0, "out_fragment");
	if (!this->linkProgram())
		return (0);
	this->deleteShaders();
	return (1);
}

void
Core::update(void)
{
}

void
Core::render(void)
{
	glUseProgram(program);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix.val);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix.val);
	ms.push();
		glUniformMatrix4fv(objLoc, 1, GL_FALSE, ms.top().val);
		glBindVertexArray(pVao);
		glBindBuffer(GL_ARRAY_BUFFER, pVbo);
	ms.pop();
	checkGlError(__FILE__, __LINE__);
}

void
Core::loop(void)
{
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		this->update();
		this->render();
		glfwSwapBuffers(this->window);
		glfwPollEvents();
	}
}
