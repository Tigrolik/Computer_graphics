#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <sstream>
#include <stdexcept>

// include glew to get OpenGL headers
#include <GL/glew.h>

class Shader {
public:
    // Ctor reads and builds the shader
    Shader(const GLchar*, const GLchar*);
    Shader(const std::string&, const std::string&);

    // get shader's program id
    GLuint id() const { return prog; }

    // use the program
    void use() const;
private:
    // The program ID
    GLuint prog;
};

#endif

