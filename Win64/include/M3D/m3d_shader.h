#ifndef M3D_SHADERS
#define M3D_SHADERS

#include <string>

class ShaderProgram
{
public:
	unsigned int programID;
	ShaderProgram(const char*, const char*);
	void fCompileAttachLink(const char*, const char*);
	inline void fUseProgram() const	{		glUseProgram(programID);	}

};

std::string fReadShader(const char*);
ShaderProgram fShaderProgramfromFile(const char* vs_path, const char* fs_path);
ShaderProgram fShaderProgramfromMemory(const int vsResourceName_, const int fsResourceName_);

#endif // !M3D_SHADERS