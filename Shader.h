#pragma once
#include <GL/glew.h>
#include <string>

namespace Utils {
	extern std::string g_glerrorStr; //�洢������Ϣ
	void printShaderLog(GLuint shader);
	void printProgramLog(int prog);
	bool checkOpenGLError(const char* function);
	std::string readShaderSource(const char* filePath);
	GLuint createShaderProgram();
}

