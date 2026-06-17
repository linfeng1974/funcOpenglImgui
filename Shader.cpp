#include "Shader.h"
#include <iostream>
#include <cstdlib>
#include <fstream>

namespace Utils {
    std::string g_glerrorStr = ""; //stores GL error info
    void printShaderLog(GLuint shader) {                           // print GLSL shader compile log
        int len = 0;
        int chWrittn = 0;
        //char* log;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            //log = (char*)malloc(len);
            std::string log;
            log.resize(len);//分配空间，c++17以上
            glGetShaderInfoLog(shader, len, &chWrittn, log.data());
            //std::cout << "Shader Info Log:" << log << std::endl;
            log.resize(chWrittn);//根据实际写入长度截断
            g_glerrorStr = "Shader Info Log: " + log;
            // free(log);
        }
    }

    void printProgramLog(int prog) {
        int len = 0;
        int chWrittn = 0;
        //char* log;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            // log = (char*)malloc(len);
            std::string log;
            log.resize(len);
            glGetProgramInfoLog(prog, len, &chWrittn, log.data());
            log.resize(chWrittn);//根据实际写入长度截断
            g_glerrorStr = "Program Info Log: " + log;
            // std::cout << "Program Info Log:" << log << std::endl;
           // free(log);
        }
    }
    bool checkOpenGLError(const char* function) {                             //���ڼ��GLSL������������openGL����ʱ����
        bool foundError = false;
        int glErr = glGetError();
        //if (glErr == 1280)return foundError;//������Чö�ٴ���
        while (glErr != GL_NO_ERROR) {
            g_glerrorStr = "glError in: " + (std::string)function + ": " + std::to_string(glErr);//?
            //std::cout << "glError in:" << function << ":" << glErr << std::endl;
            foundError = true;
            glErr = glGetError();
        }
        return foundError;
    }

    std::string readShaderSource(const char* filePath) {
        std::string content;
        std::ifstream fileStream(filePath, std::ios::in);
        std::string line = "";
        while (!fileStream.eof()) {
            std::getline(fileStream, line);
            content.append(line + "\n");
        }
        fileStream.close();
        return content;
    }


    GLuint createShaderProgram() {
        GLint vertCompiled;
        GLint fragCompiled;
        GLint linked;

        // 顶点着色器源码（你的版本）
        const char* vertexShaderSource = R"(
           #version 330 core // 使用OpenGL 3.3核心模式
           layout (location=0) in vec3 position;
           uniform mat4 mv_matrix;
           uniform mat4 proj_matrix;
           void main(void) 
          { 
              gl_Position=proj_matrix*mv_matrix*vec4(position,1.0);
           }
        )";

        // 片段着色器源码（你的版本）
        const char* fragmentShaderSource = R"(
           #version 330 core              // 使用OpenGL 3.3核心模式
           out vec4 Fragcolor; 
           uniform vec3 color;
           uniform mat4 mv_matrix;
           uniform mat4 proj_matrix;
           void main(void) 
          { Fragcolor = vec4(color,1.0f);}
        )";
        const char* vertShaderStr = vertexShaderSource;
        const char* fragShaderStr = fragmentShaderSource;
        GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vShader, 1, &vertShaderStr, NULL);
        glShaderSource(fShader, 1, &fragShaderStr, NULL);
        glCompileShader(vShader);
        checkOpenGLError("vertex shader ");
        glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
        if (vertCompiled != 1) {
            std::cout << "vertex compilation failed" << std::endl;
            printShaderLog(vShader);
        }
        glCompileShader(fShader);
        checkOpenGLError("fragshader ");
        glGetShaderiv(fShader, GL_COMPILE_STATUS, &fragCompiled);
        if (fragCompiled != 1) {
            std::cout << "fragment compilation failed" << std::endl;
            printShaderLog(fShader);
        }
        GLuint vfProgram = glCreateProgram();
        glAttachShader(vfProgram, vShader);
        glAttachShader(vfProgram, fShader);
        glLinkProgram(vfProgram);
        checkOpenGLError("vf linked ");
        glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
        if (linked != 1) {
            std::cout << "linking failed" << std::endl;
            printProgramLog(vfProgram);
        }
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        return vfProgram;
    }
}//Utils���ռ����
