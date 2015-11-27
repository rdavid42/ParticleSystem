
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
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
		core->launchKernelsAcceleration(-1, core->magnet);
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		core->launchKernelsAcceleration(1, core->magnet);

}


static void
cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	Core		*core = static_cast<Core *>(glfwGetWindowUserPointer(window));
	
	core->moveMagnet(xpos, ypos);
}


void
Core::moveMagnet(double xpos, double ypos)
{
	magnet.x =  ((xpos / windowWidth)  * 100 - 50); //100 = position Z caméra et 50 = position Z cam / 2
	magnet.y = -((ypos / windowHeight) * 100 - 50);
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

cl_int
Core::initOpencl(void)
{
	int							err;
	int							i;
	size_t						len;
	char						buffer[2048];
	static char const			*options = "-Werror -cl-fast-relaxed-math -I./inc";
	static char const			*programNames[N_PROGRAM] =
	{
		"acceleration",
		"update"
	};
	static char const			*kernelFiles[N_PROGRAM] =
	{
		"./kernels/acceleration.cl",
		"./kernels/update.cl"
	};
	std::string					file_content;
	char						*file_string;

	this->global = PARTICLE_NUMBER;
	this->num_entries = 1;
	err = clGetPlatformIDs(this->num_entries, &this->platformID, &this->num_platforms);
	if (err != CL_SUCCESS)
		return (printError(std::ostringstream().flush() << "Error: Failed to retrieve platform id ! " << err, EXIT_FAILURE));

	err = clGetDeviceIDs(this->platformID, CL_DEVICE_TYPE_GPU, 1, &this->clDeviceId, 0);
	if (err != CL_SUCCESS)
		return (printError(std::ostringstream().flush() << "Error: Failed to create a device group ! " << err, EXIT_FAILURE));

	CGLContextObj			kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj		kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties	props[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
		(cl_context_properties)kCGLShareGroup,
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)this->platformID,
		0
	};
	this->clContext = clCreateContext(props, 1, &this->clDeviceId, 0, 0, &err);
	if (!this->clContext || err != CL_SUCCESS)
		return (printError("Error: Failed to create a compute context !", EXIT_FAILURE));

	this->clCommands = clCreateCommandQueue(this->clContext, this->clDeviceId, 0, &err);
	if (!this->clCommands || err != CL_SUCCESS)
		return (printError("Error: Failed to create a command queue !", EXIT_FAILURE));

	for (i = 0; i < N_PROGRAM; ++i)
	{
		file_content = getFileContents(kernelFiles[i]);
		// std::cerr << file_content << std::endl;
		file_string = (char *)file_content.c_str();

		this->clPrograms[i] = clCreateProgramWithSource(this->clContext, 1, (char const **)&file_string, 0, &err);
		if (!this->clPrograms[i] || err != CL_SUCCESS)
		{
			return (printError(std::ostringstream().flush()
								<< "Error Failed to create compute "
								<< programNames[i]
								<< " program !",
								EXIT_FAILURE));
		}

		err = clBuildProgram(this->clPrograms[i], 0, 0, options, 0, 0);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error: Failed to build program executable ! " << err << std::endl;
			clGetProgramBuildInfo(this->clPrograms[i], this->clDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			std::cerr << buffer << std::endl;
			return (EXIT_FAILURE);
		}

		this->clKernels[i] = clCreateKernel(this->clPrograms[i], programNames[i], &err);
		if (!this->clKernels[i] || err != CL_SUCCESS)
			return (printError("Error: Failed to create compute kernel !", EXIT_FAILURE));
		err = clGetKernelWorkGroupInfo(this->clKernels[i], this->clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &this->local[i], NULL);
		if (err != CL_SUCCESS)
			return (printError(std::ostringstream().flush() << "Error: Failed to retrieve kernel work group info! " << err, EXIT_FAILURE));
		// this->local[i] /= 2;
		std::cerr << "Local workgroup size for " << programNames[i] << " kernel: " << this->local[i] << std::endl;
	}
	std::cerr << "Global workgroup size: " << this->global << std::endl;
	return (CL_SUCCESS);
}

cl_int
Core::launchKernelsAcceleration(int const &state, Vec3<float> const &pos)
{
	cl_int			err;

	glFinish();
	err = clEnqueueAcquireGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to acquire GL Objects !", EXIT_FAILURE));
	err = clSetKernelArg(clKernels[ACCELERATION_KERNEL], 0, sizeof(cl_mem), &dp);
	err |= clSetKernelArg(clKernels[ACCELERATION_KERNEL], 1, sizeof(int), &state);
	err |= clSetKernelArg(clKernels[ACCELERATION_KERNEL], 2, sizeof(float), &pos.x);
	err |= clSetKernelArg(clKernels[ACCELERATION_KERNEL], 3, sizeof(float), &pos.y);
	err |= clSetKernelArg(clKernels[ACCELERATION_KERNEL], 4, sizeof(float), &pos.z);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to set kernel arguments !", EXIT_FAILURE));
	err = clEnqueueNDRangeKernel(clCommands, clKernels[ACCELERATION_KERNEL], 1, 0, &global, &this->local[ACCELERATION_KERNEL], 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to launch acceleration kernel !", EXIT_FAILURE));
	err = clEnqueueReleaseGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release GL Objects !", EXIT_FAILURE));
	clFinish(clCommands);
/*	clock_t startTime = clock();
	err = clEnqueueReadBuffer(this->clCommands, this->dp, CL_TRUE, 0, sizeof(t_particle) * PARTICLE_NUMBER, this->hp, 0, NULL, NULL);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to read buffer !", EXIT_FAILURE));
	oss_ticks << "copy: " << (double)(clock() - startTime) << " - ";*/
	return (CL_SUCCESS);
}

cl_int
Core::launchKernelsUpdate(void)
{
	cl_int			err;

	glFinish();
	err = clEnqueueAcquireGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to acquire GL Objects !", EXIT_FAILURE));
	err = clSetKernelArg(clKernels[UPDATE_KERNEL], 0, sizeof(cl_mem), &dp);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to set kernel arguments !", EXIT_FAILURE));
	err = clEnqueueNDRangeKernel(clCommands, clKernels[UPDATE_KERNEL], 1, 0, &global, &local[UPDATE_KERNEL], 0, 0, 0);
	if (err != CL_SUCCESS)
	{
		std::cerr << "Error code: " << err << std::endl;
		return (printError("Error: Failed to launch update kernel !", EXIT_FAILURE));
	}
	err = clEnqueueReleaseGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release GL Objects !", EXIT_FAILURE));
	clFinish(clCommands);
/*	clock_t startTime = clock();
	err = clEnqueueReadBuffer(clCommands, this->dp, CL_TRUE, 0, sizeof(t_particle) * PARTICLE_NUMBER, this->hp, 0, NULL, NULL);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to read buffer !", EXIT_FAILURE));
	oss_ticks << "copy: " << (double)(clock() - startTime) << " - ";*/
	return (CL_SUCCESS);
}

cl_int
Core::initParticles(void)
{
	t_particle		*hp; // host temporary particles
	cl_int			err;

	hp = new t_particle[PARTICLE_NUMBER];
	resetParticles(hp);
	createSphere(hp);
	// OPENGL VAO/VBO INITIALISATION
	glGenVertexArrays(1, &pVao);
	glBindVertexArray(pVao);
	glGenBuffers(1, &pVbo);
	glBindBuffer(GL_ARRAY_BUFFER, pVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(t_particle) * PARTICLE_NUMBER, hp, GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(t_particle), (void *)0);
	// OPENCL INTEROPERABILITY
	dp = clCreateFromGLBuffer(clContext, CL_MEM_READ_WRITE, pVbo, &err);
	if (err != CL_SUCCESS)
		return (printError("Failed creating memory from GL buffer !", EXIT_FAILURE));
	delete [] hp;
	std::cerr << CL_SUCCESS << std::endl;
	return (CL_SUCCESS);
}

cl_int
Core::cleanDeviceMemory(void)
{
	cl_int			err;
	int				i;

	err = clReleaseMemObject(this->dp);
	if (err != CL_SUCCESS)
		return (printError("Error: Invalid device memory !", EXIT_FAILURE));
	for (i = 0; i < PARTICLE_NUMBER; ++i)
	{
		err = clReleaseProgram(this->clPrograms[i]);
		if (err != CL_SUCCESS)
			return (printError("Error: Failed to release program !", EXIT_FAILURE));
		err = clReleaseKernel(this->clKernels[i]);
		if (err != CL_SUCCESS)
			return (printError("Error: Failed to release kernel !", EXIT_FAILURE));
	}
	err = clReleaseCommandQueue(this->clCommands);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release command queue !", EXIT_FAILURE));
	err = clReleaseContext(this->clContext);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release context !", EXIT_FAILURE));
	return (CL_SUCCESS);
}
/*
cl_int
Core::writeDeviceParticles(void)
{
	cl_int			err;

	err = clEnqueueWriteBuffer(this->clCommands, this->dp, CL_TRUE, 0, sizeof(t_particle) * PARTICLE_NUMBER, this->hp, 0, NULL, NULL);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to write to source array !", EXIT_FAILURE));
	return (CL_SUCCESS);
}
*/
void
Core::resetParticles(t_particle *hp)
{
	int				i;

	for (i = 0; i < PARTICLE_NUMBER; ++i)
	{
		hp[i].pos[0] = 0;
		hp[i].pos[1] = 0;
		hp[i].pos[2] = 0;
		hp[i].vel[0] = 0;
		hp[i].vel[1] = 0;
		hp[i].vel[2] = 0;
		hp[i].acc[0] = 0;
		hp[i].acc[1] = 0;
		hp[i].acc[2] = 0;
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
Core::createSphere(t_particle *hp)
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
						this->initParticle(&hp[i], x, y, z);
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


void
Core::magnetInit(void)
{
	magnet.x = 0;
	magnet.y = 0;
	magnet.z = 0;
}

int
Core::init(void)
{
	windowWidth = 1280;
	windowHeight = 1280;
	if (!glfwInit())
		return (0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(windowWidth, windowHeight,
									"Particle System", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return (0);
	}
	glfwSetWindowUserPointer(window, this);
	glfwMakeContextCurrent(window); // make the opengl context of the window current on the main thread
	glfwSwapInterval(1); // VSYNC 60 fps max
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	buildProjectionMatrix(projMatrix, 53.13f, 0.1f, 1000.0f);
	cameraPos.set(0.0f, 0.0f, 100.0f);
	// cameraPos.set(5.5f, 5.5f, 5.5f);
	cameraLookAt.set(0.0f, 0.0f, 0.0f);
	setCamera(viewMatrix, cameraPos, cameraLookAt);
	if (initOpencl() == EXIT_FAILURE)
		return (0);
	if (!initShaders())
		return (0);
	getLocations();
	if (initParticles() != CL_SUCCESS)
		return (0);

	magnetInit();
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

int
Core::linkProgram(GLuint &program)
{
	GLint			logSize;
	GLint			state;
	char			*linkLog;

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &state);
	if (state != GL_TRUE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		linkLog = new char[logSize + 1];
		std::memset(linkLog, '\0', logSize + 1);
		glGetProgramInfoLog(program, logSize, &logSize, linkLog);
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
	if (!(program = glCreateProgram()))
		return (printError("Failed to create program !", 0));
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glBindFragDataLocation(program, 0, "out_fragment");
	if (!linkProgram(program))
		return (0);
	checkGlError(__FILE__, __LINE__);
	deleteShaders();
	return (1);
}

void
Core::update(void)
{

 	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		launchKernelsAcceleration(1, magnet);
	else
		launchKernelsUpdate();

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
		glDrawArrays(GL_POINTS, 0, PARTICLE_NUMBER);
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
