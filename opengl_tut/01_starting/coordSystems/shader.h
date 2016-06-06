#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// include glew to get OpenGL headers
#include <GL/glew.h>

class Shader {
    public:
        // The program ID
        GLuint Program;
        // Ctor reads and builds the shader
        Shader(const GLchar *vertexPath, const GLchar *fragmentPath);
        // use the program
        void Use();
};

#endif
