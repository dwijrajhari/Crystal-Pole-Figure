#include <fstream>
#include <sstream>

#include <M3D/m3d_res.h>
#include <M3D/m3d_basics.h>
#include <M3D/m3d_shader.h>

#include "resource.h"

ShaderProgram::ShaderProgram(const char* VS, const char* FS)
	: programID(glCreateProgram())
{
	//fCompileAttachLink(fReadShader(VS).c_str(), fReadShader(FS).c_str());
	fCompileAttachLink(VS, FS);

}

std::string fReadShader(const char* filepath)
{
	std::string line;
	std::ifstream shaderIn;
	std::stringstream shaderOut;
	shaderIn.open(filepath);
	while (std::getline(shaderIn, line))
	{
		shaderOut << line << "\n";
	}
	shaderIn.close();
	LogMan(0, "Using Shader:");
	LogMan(0, filepath);
	return shaderOut.str();
}

void ShaderProgram::fCompileAttachLink(const char* VS, const char* FS)
{
	int errorCode = 0;
	GLchar infoLog[512];

	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VS, NULL);
	glCompileShader(VertexShader);

	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &errorCode);
	if (!errorCode) {
		glGetShaderInfoLog(VertexShader, 512, NULL, infoLog);
		LogMan(2, infoLog);
	}
	else {
		LogMan(0, "VertexShader compiled");
	}

	glAttachShader(programID, VertexShader);
	glDeleteShader(VertexShader);

	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FS, NULL);
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &errorCode);
	if (!errorCode) {
		glGetShaderInfoLog(FragmentShader, 512, NULL, infoLog);
		LogMan(2, infoLog);
	}
	else {
		LogMan(0, "FragmentShader compiled");
	}

	glAttachShader(programID, FragmentShader);
	glDeleteShader(FragmentShader);

	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &errorCode);
	if (!errorCode) {
		glGetProgramInfoLog(programID, 512, NULL, infoLog);
		LogMan(2, infoLog);
	}
	else {
		LogMan(0, "Shaderprogram linked and ready for use");

	}
}

ShaderProgram fShaderProgramfromFile(const char* vs_path, const char* fs_path)
{
	return ShaderProgram(fReadShader(vs_path).c_str(), fReadShader(fs_path).c_str());
}

ShaderProgram fShaderProgramfromMemory(const int vsResourceName_, const int fsResourceName_)
{
	unsigned int vshaderSize = fGetResourceSize(vsResourceName_, SHADER) + 1;
	unsigned int fshaderSize = fGetResourceSize(fsResourceName_, SHADER) + 1;
	char* vshader = new char[vshaderSize]();
	char* fshader = new char[fshaderSize]();
	fGetResourceData(vsResourceName_, SHADER, vshader);
	fGetResourceData(fsResourceName_, SHADER, fshader);
	ShaderProgram shaderprogram = ShaderProgram(vshader, fshader);
	delete[] vshader;
	delete[] fshader;
	return shaderprogram;
}








