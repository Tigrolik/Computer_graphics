#include <iostream>
#include "Shader.h"

/*
 * helper functions
 */
// read shader from a file into a string
std::string read_shader_file(const GLchar *shad_file) {
    using namespace std;
    ifstream ifs {shad_file};
    if (!ifs)
        throw runtime_error {"cannot open file " + string(shad_file)};
    ifs.exceptions(ifs.exceptions() | ios_base::badbit);
    // keep the result in a string
    string shad_str {};
    for (string s; getline(ifs, s);) {
        if (ifs.fail()) {
            ifs.unget();
            ifs.clear(ios_base::failbit);
        }
        shad_str += s + '\n';
    }
    return shad_str;
}

// Create a shader object
GLuint init_shader(const GLenum shad_type, const GLchar *src) {
    // parameter: provide the type of shader
    GLuint shad = glCreateShader(shad_type);
    // attach the shader source code to the shader object
    glShaderSource(shad, 1, &src, nullptr);
    // compile the shader
    glCompileShader(shad);
    // check if compilation was successful
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(shad, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shad, 512, nullptr, info_log);
        throw std::runtime_error {"Shader compilation failed:\n" +
                std::string(info_log)};
    }
    return shad;
}


// constructor
Shader::Shader(const GLchar *vert, const GLchar *frag): prog{} {
    // get vertex and fragment shaders
    std::string vert_src {read_shader_file(vert)};
    std::string frag_src {read_shader_file(frag)};
    const GLuint vert_shad {init_shader(GL_VERTEX_SHADER, vert_src.c_str())};
    const GLuint frag_shad {init_shader(GL_FRAGMENT_SHADER, frag_src.c_str())};
    // shader program
    prog = glCreateProgram();
    // attach the compiled shaders to the program
    glAttachShader(prog, vert_shad);
    glAttachShader(prog, frag_shad);
    // link the shaders
    glLinkProgram(prog);
    // check if linking the shader program failed
    GLint success;
    GLchar info_log[512];
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, nullptr, info_log);
        throw std::runtime_error {"Shader program linking failed:\n" +
                std::string(info_log)};
    }
    glDeleteShader(vert_shad);
    glDeleteShader(frag_shad);
}

// constructor using std::string
Shader::Shader(const std::string &vert, const std::string &frag):
    Shader(vert.c_str(), frag.c_str()) {
    }

void Shader::use() const {
    glUseProgram(prog);
}

