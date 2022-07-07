#include <iostream>
#include <M3D/m3d.h>

M3D::M3D(GLuint SW = 800, GLuint SH = 800, const char* WinName = "M3D")
	:mScreenWidth(SW), mScreenHeight(SH), mWinName(WinName)
{
	if (!glfwInit())
	{
		LogMan(2, "GLFW init failed");
	}
	else
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);
		//glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		glfwSetErrorCallback(LogMan);
		mWindow = glfwCreateWindow(SW, SH, WinName, NULL, NULL);
	}

	if (!mWindow)
	{
		glfwTerminate();
		LogMan(2, "Window creation failed");
	}
	else
	{
		glfwMakeContextCurrent(mWindow);
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			LogMan(2, "GLEW init failed");
		}
		else
		{
			std::cout << "Using " << glGetString(GL_RENDERER) << " by " << glGetString(GL_VENDOR) << std::endl;
			LogMan(0, "M3D setup was successful");
		}

	}
}

bool M3D::fIsRunning()
{
	return !glfwWindowShouldClose(mWindow);
}

void M3D::fClearScreen(float r = 0.0f, float g = 0.0f, float b = 0.0f)
{
	glClearColor(r, g, b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void M3D::fUpdateState()
{
	glfwSwapBuffers(mWindow);
	glfwPollEvents();
}

void M3D::fQuit()
{
	glfwTerminate();
	LogMan(0, "M3D has now closed.\n");
}







