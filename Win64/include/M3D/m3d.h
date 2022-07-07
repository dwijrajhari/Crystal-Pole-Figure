#ifndef M3D_MAIN
#define M3D_MAIN

#include <M3D/m3d_basics.h>

class M3D
{
public:
	GLFWwindow* mWindow = NULL;
	const char* mWinName;
	GLuint mScreenWidth, mScreenHeight;

	M3D(GLuint, GLuint, const char*);
	bool fIsRunning();
	void fClearScreen(float, float, float);
	void fUpdateState();
	void fQuit();

};


#endif // !M3D


