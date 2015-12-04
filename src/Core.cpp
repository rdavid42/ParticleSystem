
#include "Core.hpp"

Core::Core(void)
{
}

Core::~Core(void)
{
	cleanDeviceMemory();
	glfwDestroyWindow(window);
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
	{
		if (core->gravity)
			core->gravity = !core->gravity;
		core->launchKernelsAcceleration(-1, core->magnet);
	}
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		core->gravity = !core->gravity;
		if (core->gravity)
			core->gravityPos.set(core->magnet.x,
								core->magnet.y,
								core->magnet.z);
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS && !core->emitterActive)
		core->launchKernelsResetShape(MAKESPHERE_KERNEL);
	if (key == GLFW_KEY_2 && action == GLFW_PRESS && !core->emitterActive)
		core->launchKernelsResetShape(MAKECUBE_KERNEL);
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		core->launchKernelReset();
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		core->launchKernelReset();
		core->emitterActive = !core->emitterActive;
	}
	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
	{
		core->particleSize += core->particleSizeInc;
		if (core->particleSize > core->particleSizeMax)
			core->particleSize = core->particleSizeMax;
		glPointSize(core->particleSize);
	}
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
	{
		core->particleSize -= core->particleSizeInc;
		if (core->particleSize < core->particleSizeMin)
			core->particleSize = core->particleSizeMin;
		glPointSize(core->particleSize);
	}
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
	magnet.x =  ((xpos / windowWidth)  * cameraPos.z - cameraPos.z / 2); //100 = position Z caméra et 50 = position Z cam / 2
	magnet.y = -((ypos / windowHeight) * cameraPos.z - cameraPos.z / 2);
}

void
Core::buildProjectionMatrix(Mat4<float> &proj, float const &fov,
							float const &near, float const &far)
{
	float const			f = 1.0f / tan(fov * (M_PI / 360.0));
	float const			ratio = (1.0f * windowWidth) / windowHeight;

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
	setViewMatrix(view, dir, right, up);
	translation.setTranslation(-pos.x, -pos.y, -pos.z);
	view.multiply(translation);
}

cl_int
Core::getOpenCLInfo(void)
{
	static int const	size = 2048;
	char				buffer[size];
	cl_int				err;

	err = clGetPlatformInfo(platformID, CL_PLATFORM_VERSION, size, buffer, 0);
	if (err != CL_SUCCESS)
		std::cerr << "Failed to retrieve platform version!" << std::endl;
	else
		std::cerr << "VERSION: " << buffer << std::endl;
	err = clGetPlatformInfo(platformID, CL_PLATFORM_NAME, size, buffer, 0);
	if (err != CL_SUCCESS)
		std::cerr << "Failed to retrieve platform name!" << std::endl;
	else
		std::cerr << buffer << std::endl;
	err = clGetPlatformInfo(platformID, CL_PLATFORM_VENDOR, size, buffer, 0);
	if (err != CL_SUCCESS)
		std::cerr << "Failed to retrieve platform vendor!" << std::endl;
	else
		std::cerr << buffer << std::endl;
	err = clGetPlatformInfo(platformID, CL_PLATFORM_EXTENSIONS, size, buffer, 0);
	if (err != CL_SUCCESS)
		std::cerr << "Failed to retrieve platform extensions!" << std::endl;
	else
		std::cerr << buffer << std::endl;
	return (CL_SUCCESS);
}

cl_int
Core::initOpencl(void)
{
	int							err;
	int							i;
	size_t						len;
	char						buffer[2048];
	static char const			*options = "-I./inc -I./kernels -Werror -cl-fast-relaxed-math";
	static char const			*programNames[N_PROGRAM] =
	{
		"acceleration",
		"update",
		"makeCube",
		"makeSphere",
		"reset",
		"sprayEmitter"
	};
	static char const			*kernelFiles[N_PROGRAM] =
	{
		"./kernels/acceleration.cl",
		"./kernels/update.cl",
		"./kernels/makeCube.cl",
		"./kernels/makeSphere.cl",
		"./kernels/reset.cl",
		"./kernels/sprayEmitter.cl"
	};
	std::string					file_content;
	char						*file_string;

	global = PARTICLE_NUMBER;
	num_entries = 1;
	err = clGetPlatformIDs(num_entries, &platformID, &num_platforms);
	if (err != CL_SUCCESS)
		return (printError(std::ostringstream().flush() << "Error: Failed to retrieve platform id ! " << err, EXIT_FAILURE));

	err = clGetDeviceIDs(platformID, CL_DEVICE_TYPE_GPU, 1, &clDeviceId, 0);
	if (err != CL_SUCCESS)
		return (printError(std::ostringstream().flush() << "Error: Failed to create a device group ! " << err, EXIT_FAILURE));

#ifdef __APPLE__
	CGLContextObj			kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj		kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties	props[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
		(cl_context_properties)kCGLShareGroup,
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platformID,
		0
	};
#else
	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR,
		(cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR,
		(cl_context_properties)glXGetCurrentDisplay(),
		0
	};
#endif
	clContext = clCreateContext(props, 1, &clDeviceId, 0, 0, &err);
	if (!clContext || err != CL_SUCCESS)
		return (printError("Error: Failed to create a compute context !", EXIT_FAILURE));
	clCommands = clCreateCommandQueue(clContext, clDeviceId, 0, &err);
	if (!clCommands || err != CL_SUCCESS)
		return (printError("Error: Failed to create a command queue !", EXIT_FAILURE));
	for (i = 0; i < N_PROGRAM; ++i)
	{
		file_content = getFileContents(kernelFiles[i]);
		file_string = (char *)file_content.c_str();
		clPrograms[i] = clCreateProgramWithSource(clContext, 1, (char const **)&file_string, 0, &err);
		if (!clPrograms[i] || err != CL_SUCCESS)
		{
			return (printError(std::ostringstream().flush()
								<< "Error Failed to create compute "
								<< programNames[i]
								<< " program !",
								EXIT_FAILURE));
		}
		err = clBuildProgram(clPrograms[i], 0, 0, options, 0, 0);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error: Failed to build program executable ! " << err << std::endl;
			clGetProgramBuildInfo(clPrograms[i], clDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			std::cerr << buffer << std::endl;
			return (EXIT_FAILURE);
		}
		clKernels[i] = clCreateKernel(clPrograms[i], programNames[i], &err);
		if (!clKernels[i] || err != CL_SUCCESS)
			return (printError("Error: Failed to create compute kernel !", EXIT_FAILURE));
		err = clGetKernelWorkGroupInfo(clKernels[i], clDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &local[i], NULL);
		if (err != CL_SUCCESS)
			return (printError(std::ostringstream().flush() << "Error: Failed to retrieve kernel work group info! " << err, EXIT_FAILURE));
		// std::cerr << "Local workgroup size for " << programNames[i] << " kernel: " << local[i] << std::endl;
	}
	emitter_global = local[SPRAY_EMITTER_KERNEL];
	// std::cerr << "Global workgroup size: " << global << std::endl;
	return (CL_SUCCESS);
}

cl_int
Core::launchKernelsResetShape(int kernel)
{
	cl_int		err;
	int			m = int(std::cbrt(PARTICLE_NUMBER) / 2);

	err = clEnqueueAcquireGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to acquire GL Objects !", EXIT_FAILURE));
	err = clSetKernelArg(clKernels[kernel], 0, sizeof(cl_mem), &dp);
	err |= clSetKernelArg(clKernels[kernel], 1, sizeof(int), &m);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to set kernel arguments !", EXIT_FAILURE));
	err = clEnqueueNDRangeKernel(clCommands, clKernels[kernel], 1, 0, &global, &local[kernel], 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to launch reset shape kernel !", EXIT_FAILURE));
	err = clEnqueueReleaseGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release GL Objects !", EXIT_FAILURE));
	clFinish(clCommands);
	return (CL_SUCCESS);
}

cl_int
Core::launchKernelReset(void)
{
	cl_int		err;

	err = clEnqueueAcquireGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to acquire GL Objects !", EXIT_FAILURE));
	err = clSetKernelArg(clKernels[RESET_KERNEL], 0, sizeof(cl_mem), &dp);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to set kernel arguments !", EXIT_FAILURE));
	err = clEnqueueNDRangeKernel(clCommands, clKernels[RESET_KERNEL], 1, 0, &global, &local[RESET_KERNEL], 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to launch reset kernel !", EXIT_FAILURE));
	err = clEnqueueReleaseGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release GL Objects !", EXIT_FAILURE));
	clFinish(clCommands);
	emitter_global = local[SPRAY_EMITTER_KERNEL];
	return (CL_SUCCESS);
}

cl_int
Core::launchKernelSprayEmitter(void)
{
	cl_int			err;
	Vec3<float>		emitter(0.0f, 0.0f, 0.0f);

	err = clEnqueueAcquireGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to acquire GL Objects !", EXIT_FAILURE));
	err = clSetKernelArg(clKernels[SPRAY_EMITTER_KERNEL], 0, sizeof(cl_mem), &dp);
	err |= clSetKernelArg(clKernels[SPRAY_EMITTER_KERNEL], 1, sizeof(cl_float) * 3, &emitter);
	err |= clSetKernelArg(clKernels[SPRAY_EMITTER_KERNEL], 2, sizeof(cl_float) * 3, &magnet);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to set kernel arguments !", EXIT_FAILURE));
	err = clEnqueueNDRangeKernel(clCommands, clKernels[SPRAY_EMITTER_KERNEL], 1, 0, &emitter_global, &local[SPRAY_EMITTER_KERNEL], 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to launch reset kernel !", EXIT_FAILURE));
	err = clEnqueueReleaseGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release GL Objects !", EXIT_FAILURE));
	clFinish(clCommands);
	if (emitter_global < global)
		emitter_global += local[SPRAY_EMITTER_KERNEL] * 3;
	return (CL_SUCCESS);
}

cl_int
Core::launchKernelsAcceleration(int const &state, Vec3<float> const &pos)
{
	cl_int			err;

	err = clEnqueueAcquireGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to acquire GL Objects !", EXIT_FAILURE));
	err = clSetKernelArg(clKernels[ACCELERATION_KERNEL], 0, sizeof(cl_mem), &dp);
	err |= clSetKernelArg(clKernels[ACCELERATION_KERNEL], 1, sizeof(int), &state);
	err |= clSetKernelArg(clKernels[ACCELERATION_KERNEL], 2, sizeof(float) * 3, &pos);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to set kernel arguments !", EXIT_FAILURE));
	err = clEnqueueNDRangeKernel(clCommands, clKernels[ACCELERATION_KERNEL], 1, 0, &global, &local[ACCELERATION_KERNEL], 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to launch acceleration kernel !", EXIT_FAILURE));
	err = clEnqueueReleaseGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release GL Objects !", EXIT_FAILURE));
	clFinish(clCommands);
	return (CL_SUCCESS);
}

cl_int
Core::launchKernelsUpdate(void)
{
	cl_int			err;

	err = clEnqueueAcquireGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to acquire GL Objects !", EXIT_FAILURE));
	err = clSetKernelArg(clKernels[UPDATE_KERNEL], 0, sizeof(cl_mem), &dp);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to set kernel arguments !", EXIT_FAILURE));
	err = clEnqueueNDRangeKernel(clCommands, clKernels[UPDATE_KERNEL], 1, 0, &global, &local[UPDATE_KERNEL], 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to launch update kernel !", EXIT_FAILURE));
	err = clEnqueueReleaseGLObjects(clCommands, 1, &dp, 0, 0, 0);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release GL Objects !", EXIT_FAILURE));
	clFinish(clCommands);
	return (CL_SUCCESS);
}

cl_int
Core::cleanDeviceMemory(void)
{
	cl_int			err;
	int				i;

	err = clReleaseMemObject(dp);
	if (err != CL_SUCCESS)
		return (printError("Error: Invalid device memory !", EXIT_FAILURE));
	for (i = 0; i < N_PROGRAM; ++i)
	{
		err = clReleaseProgram(clPrograms[i]);
		if (err != CL_SUCCESS)
			return (printError("Error: Failed to release program !", EXIT_FAILURE));
		err = clReleaseKernel(clKernels[i]);
		if (err != CL_SUCCESS)
			return (printError("Error: Failed to release kernel !", EXIT_FAILURE));
	}
	err = clReleaseCommandQueue(clCommands);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release command queue !", EXIT_FAILURE));
	err = clReleaseContext(clContext);
	if (err != CL_SUCCESS)
		return (printError("Error: Failed to release context !", EXIT_FAILURE));
	return (CL_SUCCESS);
}

void
Core::getLocations(void)
{
	// attribute variables
	positionLoc = glGetAttribLocation(this->program, "position");
	lifeLoc = glGetAttribLocation(this->program, "life");
	// uniform variables
	redLoc = glGetUniformLocation(this->program, "red");
	greenLoc = glGetUniformLocation(this->program, "green");
	blueLoc = glGetUniformLocation(this->program, "blue");
	rredLoc = glGetUniformLocation(this->program, "rred");
	rgreenLoc = glGetUniformLocation(this->program, "rgreen");
	rblueLoc = glGetUniformLocation(this->program, "rblue");
	projLoc = glGetUniformLocation(this->program, "proj_matrix");
	viewLoc = glGetUniformLocation(this->program, "view_matrix");
	colorLoc = glGetUniformLocation(this->program, "frag_color");
	objLoc = glGetUniformLocation(this->program, "obj_matrix");
	emitterActiveLoc = glGetUniformLocation(this->program, "emitterActive");
}

GLuint
Core::loadTexture(char const *filename)
{
	GLuint				texture;
	Bmp					bmp;

	if (!bmp.load(filename))
		return (printError("Failed to load bmp !", 0));
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp.width, bmp.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	checkGlError(__FILE__, __LINE__);
	return (texture);
}

void
Core::magnetInit(void)
{
	magnet.x = 0;
	magnet.y = 0;
	magnet.z = 0;
}

void
glErrorCallback(GLenum        source,
				GLenum        type,
				GLuint        id,
				GLenum        severity,
				GLsizei       length,
				const GLchar* message,
				GLvoid*       userParam)
{
	(void)userParam;
	(void)length;
	std::cerr << "OpenGL Error:" << std::endl;
	std::cerr << "=============" << std::endl;
	std::cerr << " Object ID: " << id << std::endl;
	std::cerr << " Severity:  " << severity << std::endl;
	std::cerr << " Type:      " << type << std::endl;
	std::cerr << " Source:    " << source << std::endl;
	std::cerr << " Message:   " << message << std::endl;
	glFinish();
}

int
Core::init(void)
{

	windowWidth = 2560;
	windowHeight = 1440;
	if (!glfwInit())
		return (0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(windowWidth, windowHeight, "Particle System", NULL, NULL);
// 	window = glfwCreateWindow(windowWidth, windowHeight,
// 									"Particle System", glfwGetPrimaryMonitor(), NULL);
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	buildProjectionMatrix(projMatrix, 53.13f, 0.1f, 1000.0f);
	cameraPos.set(0.0f, 0.0f, 200.0f);
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
	particleSize = 1.0;
	particleSizeInc = 1.0;
	particleSizeMin = 1.0;
	particleSizeMax = 10.0;
	glPointSize(particleSize);
	gravity = 0;
	return (1);
}

cl_int
Core::initParticles(void)
{
	GLint			bsize;
	cl_int			err;

	// std::cerr << glGetString(GL_VENDOR) << std::endl;
	// std::cerr << glGetString(GL_RENDERER) << std::endl;
	// std::cerr << glGetString(GL_VERSION) << std::endl;
	// std::cerr << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	// OPENGL VAO/VBO INITIALISATION
#ifndef __APPLE__
	if (glDebugMessageControlARB != NULL)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallbackARB((GLDEBUGPROCARB)glErrorCallback, NULL);
	}
#endif
	glGenVertexArrays(1, &pVao);
	glBindVertexArray(pVao);
	glGenBuffers(1, &pVbo);
	glBindBuffer(GL_ARRAY_BUFFER, pVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(t_particle) * PARTICLE_NUMBER, 0, GL_STATIC_DRAW);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bsize);
	// std::cerr << "VBO size: " << bsize << std::endl;
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(t_particle), (void *)0);
	glEnableVertexAttribArray(lifeLoc);
	glVertexAttribPointer(lifeLoc, 1, GL_FLOAT, GL_FALSE, sizeof(t_particle), (void *)(sizeof(float) * 9));
	// OPENCL INTEROPERABILITY
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	dp = clCreateFromGLBuffer(clContext, CL_MEM_READ_WRITE, pVbo, &err);
	if (err != CL_SUCCESS)
		return (printError("Failed creating memory from GL buffer !", EXIT_FAILURE));
	launchKernelsResetShape(MAKESPHERE_KERNEL);
	// texture
	checkGlError(__FILE__, __LINE__);
	emitterActive = false;
	return (CL_SUCCESS);
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
	if (!(vertexShader = loadShader(GL_VERTEX_SHADER, "./shaders/vertex_shader.gls")))
		return (printError("Failed to load vertex shader !", 0));
	if (!(fragmentShader = loadShader(GL_FRAGMENT_SHADER, "./shaders/fragment_shader.gls")))
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
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
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
	if (emitterActive)
	{
		launchKernelSprayEmitter();
	}
	else
	{
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (gravity)
				gravity = !gravity;
			launchKernelsAcceleration(1, magnet);
		}
		else if (gravity == 1)
			launchKernelsAcceleration(1, gravityPos);
		else
			launchKernelsUpdate();
	}
}

void
Core::render(void)
{
	float		ftime = glfwGetTime();

	glUseProgram(program);
	glUniform1f(emitterActiveLoc, float(emitterActive));
	if (gravity)
	{
		glUniform1f(redLoc, (gravityPos.x));
		glUniform1f(greenLoc, (gravityPos.y));
		glUniform1f(blueLoc, (gravityPos.z));
	}
	else
	{
		glUniform1f(redLoc, (magnet.x));
		glUniform1f(greenLoc, (magnet.y));
		glUniform1f(blueLoc, (magnet.z));
	}
 	glUniform1f(rredLoc, (std::abs(-0.5 + cos(ftime * 0.4 + 1.5))));
 	glUniform1f(rgreenLoc, std::abs(-0.5 + cos(ftime * 0.6) * sin(ftime * 0.3)));
 	glUniform1f(rblueLoc, (std::abs(-0.5 + sin(ftime * 0.2))));
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
	double		lastTime, currentTime;
	double		frames;

	frames = 0.0;
	lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		frames += 1.0;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		update();
		render();
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (currentTime - lastTime >= 1.0)
		{
			glfwSetWindowTitle(window, std::to_string(1000.0 / frames).c_str());
			frames = 0.0;
			lastTime += 1.0;
		}
	}
}
