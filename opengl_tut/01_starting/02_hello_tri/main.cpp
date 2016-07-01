/*
 * Following tutorial on OpenGL:
 * http://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * In this program we see how to draw a triangle in OpenGL.
 * The approach have subtle differences from the tutorial (e.g., using Bad_init
 * struct for throwing exceptions or dividing the code into smaller functions)
 */

#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/*
 * Shaders... for this first time the shaders are defined right here, in the
 * code. Next time they will be put in separate file(s)
 */
static const GLchar *vert_shad_src {
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "void main()\n"
    "{\n"
    "gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
    "}\0"
};

static const GLchar *frag_shad_src {
    "#version 330 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(0.85f, 0.71f, 0.33f, 1.0f);\n"
    "}\0"
};

static const GLchar *new_frag_shad_src {
    "#version 330 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(0.48f, 0.91f, 0.2f, 1.0f);\n"
    "}\0"
};

/*
 * Decided to use (also for practicing reasons) an exception.
 * This exception is to be thrown when, for example, the window is not
 * successfully created
 */
struct Bad_init: std::runtime_error {
    explicit Bad_init(const std::string &s): std::runtime_error{s} { }
    explicit Bad_init(const char *s): std::runtime_error{s} { }
};

/*
 * Another thing used here is functions declarations: not to have the main()
 * function to deep in the code
 */
// initialize stuff
GLFWwindow* init(const GLuint, const GLuint);
// callback function
void key_callback(GLFWwindow*, const int, const int, const int, const int);
// drawing loop
void game_loop(GLFWwindow*, const GLuint*, const GLuint, const GLuint*,
       const GLuint, const GLuint, const GLuint, const int);
// cleaning up
int clean_up(const int);
// --- shader functions ---
GLuint make_shader(const GLenum, const GLchar*);    // create a shader
GLuint shader_program(const GLuint, const GLuint);  // create a shader program
// generate VAO, VBO, EBO...
void gen_objects(GLuint*, GLuint*);
void gen_objects(GLuint*, GLuint*, GLuint*);
// bind the objects with vertex data
void make_objects(const GLuint, const GLuint, const GLfloat*, const GLuint);
void make_objects(const GLuint, const GLuint, const GLfloat*, const GLuint,
        const GLuint*, const GLuint);

// --- functions showing the examples of drawing triangle ---
void draw_triangle(GLFWwindow*, const GLuint);
void draw_two_triangles(GLFWwindow*, const GLuint);
void drawing_triangle(GLFWwindow*, const GLuint, const GLfloat*, const GLuint);
void draw_two_triangles2(GLFWwindow*, const GLuint*, const GLuint);
void draw_two_triangles3(GLFWwindow*);

// --- functions showing the examples of drawing rectangle ---
void draw_rect(GLFWwindow*, const GLuint);
void drawing_rect(GLFWwindow*, const GLuint, const GLfloat*, const GLuint,
        const GLuint*, const GLuint);

// here goes the main()
int main(int argc, char *argv[]) try {
    static constexpr GLuint width {800}, height {600};
    // make a window
    GLFWwindow *win = init(width, height);
    /*
     * Build a shader program from two shaders: vertex and fragment.
     * The corresponding shaders are given as GLchar*
     */
    GLuint shad_prog = shader_program(
            make_shader(GL_VERTEX_SHADER, vert_shad_src),
            make_shader(GL_FRAGMENT_SHADER, frag_shad_src)
            );
    static constexpr char num_options {'4'};
    if (argc > 1) {
        const std::string s {argv[1]};
        const char inp_char {s[0]};
        if (s.length() == 1 && inp_char >= '0' && inp_char <= num_options) {
            switch (inp_char - '0') {
                case 1:
                    draw_rect(win, shad_prog);
                    break;
                case 2:
                    draw_two_triangles(win, shad_prog);
                    break;
                case 3:
                    draw_two_triangles2(win, &shad_prog, 1);
                    break;
                case 4:
                    draw_two_triangles3(win);
                    break;
                case 0:
                default:
                    draw_triangle(win, shad_prog);
            }
        } else {
            std::cerr << "Wrong input: drawing default triangle\n";
            draw_triangle(win, shad_prog);
        }
    } else {
        std::cout << "Note: the program can be used as follows:\n" <<
            argv[0] << " int_param, where int_param can be:\n" <<
            "0:\t triangle (default)\n" << "1:\t rectangle\n" <<
            "2:\t two triangles\n3:\t two triangle (two VAOs and VBOs)\n" <<
            "4:\t two triangles (different shaders)\n";
        draw_triangle(win, shad_prog);
    }
    // clean up and exit properly
    return clean_up(0);
} catch (const Bad_init &e) {
    std::cerr << e.what() << '\n';
    return clean_up(1);
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return clean_up(2);
} catch (...) {
    std::cerr << "Unknown exception\n";
    return clean_up(3);
}

/*
 * Function to initialize stuff. With comparison to the first tutorial here we
 * put more stuff inside a single function, though trying not to make it too
 * long (keeping the idea of fitting a single function on a screen in mind)
 */
GLFWwindow* init(const GLuint w, const GLuint h) {
    glfwInit();

    // --- configuring GLFW ---
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // --- create a window object
    GLFWwindow *win = glfwCreateWindow(w, h, "Triangle", nullptr, nullptr);
    if (win == nullptr)
        throw Bad_init {"Failed to create GLFW window"};

    glfwMakeContextCurrent(win);

    // let GLEW know to use a modern approach to retrieving function pointers
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw Bad_init {"Failed to initialize GLEW"};

    // register the callback
    glfwSetKeyCallback(win, key_callback);

    // inform OpenGL about the size of the rendering window
    glViewport(0, 0, w, h);

    return win;
}

/*
 * Loop for keeping the window drawing stuff - calling it a game loop.
 * Last parameter is used here as a switch to choose either to draw the example
 * triangle or rectangle
 */
void game_loop(GLFWwindow *win, const GLuint *VAO, const GLuint VAO_size,
        const GLuint *shad_prog, const GLuint prog_size, const GLuint arr_size,
        const int option) {
    // polygon drawing mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // check if GLFW has been told to close
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents(); // check for triggered events
        // --- Rendering happens here ---
        // Set the color, clear the color buffer
        glClearColor(0.33f, 0.44f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // drawing
        for (GLuint i = 0; i < VAO_size; ++i) {
            if (prog_size > 1)
                glUseProgram(shad_prog[i]);
            else
                glUseProgram(shad_prog[0]);
            glBindVertexArray(VAO[i]);
            switch (option) {
                case 1: // rectangle
                    glDrawElements(GL_TRIANGLES, arr_size / sizeof(GLuint),
                            GL_UNSIGNED_INT, 0);
                    break;
                case 0: // triangle(s)
                default:
                    glDrawArrays(GL_TRIANGLES, 0, arr_size / sizeof(GLfloat));
            }
        }
        glBindVertexArray(0);

        // swap the screen buffers
        glfwSwapBuffers(win);
    }
}

/*
 * Call this function whenever a key is pressed / released
 */
void key_callback(GLFWwindow *win, const int key, const int,
        const int action, const int) {
    // when the escape key is pressed set the WindowShouldClose to true
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GL_TRUE);
}

/*
 * Properly clean up and return the value as an indicator of success (0) or
 * failure (1 or other non-zero value)
 */
int clean_up(const int val) {
    glfwTerminate();
    return val;
}

/*
 * Create a shader object
 */
GLuint make_shader(const GLenum shad_type, const GLchar *src) {
    // parameter: provide the type of shader
    GLuint shad = glCreateShader(shad_type);
    // attach the shader source code to the shader object
    glShaderSource(shad, 1, &src, nullptr);
    // compile the shader
    glCompileShader(shad);
    // check if compilation was successful
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shad, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shad, 512, nullptr, infoLog);
        throw Bad_init {infoLog};
    }
    return shad;
}

/*
 * In order to use shaders we need to link them to a shader program object
 * and then activate the program when rendering objects.
 * Create a program and return the ID ref to the program object
 */
GLuint shader_program(const GLuint vert_shad, const GLuint frag_shad) {
    GLuint shad_prog = glCreateProgram();
    // attach the compiled shaders to the program
    glAttachShader(shad_prog, vert_shad);
    glAttachShader(shad_prog, frag_shad);
    // link the shaders
    glLinkProgram(shad_prog);
    // check if linking the shader program failed
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shad_prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shad_prog, 512, nullptr, infoLog);
        throw Bad_init {infoLog};
    }
    /*
     * Now every shader and rendering call will use this program object and,
     * therefore, the shaders.
     * Delete the shaders objects once they have been linked into the program
     * object
     */
    glDeleteShader(vert_shad);
    glDeleteShader(frag_shad);
    return shad_prog;
}

/*
 * Generate objects like Vertex Array Objects, Vertex Buffer Object
 */
void gen_objects(GLuint *VAO, GLuint *VBO) {
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
}
// version with Element Buffer Object
void gen_objects(GLuint *VAO, GLuint *VBO, GLuint *EBO) {
    gen_objects(VAO, VBO);
    glGenBuffers(1, EBO);
}

/*
 * Bind objects with vertex data. This function has a lot of comment thus is
 * long
 */
void make_objects(const GLuint VAO, const GLuint VBO,
        const GLfloat *vertices, const GLuint vert_size) {
    glBindVertexArray(VAO); // bind the VAO with the prefered settings
    /*
     * Copy vertices array in a buffer for OpenGL to use.
     * bind the buffer to GL_ARRAY_BUFFER: buffer type of VBO
     */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /*
     * Now every buffer call will be used to configure the currently bound
     * buffer: VBO.
     * copy the previously defined vertex data into buffer's memory
     */
    glBufferData(GL_ARRAY_BUFFER, vert_size, vertices, GL_STATIC_DRAW);

    /*
     * Tell OpenGl how to interpret the vertex data (per vertex attribute)
     * parameters:
     * 1) which vertex attribute to configure (location = 0 in our example)
     * 2) size of vertex attribute: vec3 => attr is composed of 3 values
     * 3) type of data
     * 4) whether to normalize the data or not
     * 5) stride = space between consecutive vertex attribute sets (3 times the
     *    size of GLfloat and the array is tightly packed)
     * 6) offset of where the position data begins in the buffer
     */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
            (GLvoid*)0);
    /*
     * Enable the vertex attribute: since the defined VBO was bound before,
     * then the vertex attribute 0 is now associated with its vertex data
     */
    glEnableVertexAttribArray(0);

    /*
     * The next is allowed: the call to glVertexAttribPointer registered VBO as
     * the currently bound vertex buffer object so afterwards we can safely
     * unbind
     */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /*
     * Unbind VAO (it is always good to unbind any buffer/array to prevent
     * strange bugs). Do not unbind the EBO, keep it bound to this VAO
     */
    glBindVertexArray(0);
}

/*
 * Version of the previous function including EBO: requires indices and their
 * size
 */
void make_objects(const GLuint VAO, const GLuint VBO, const GLuint EBO,
        const GLfloat *vertices, const GLuint vert_size,
        const GLuint *indices, const GLuint ind_size) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vert_size, vertices, GL_STATIC_DRAW);

    // bind EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind_size, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
            (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/*
 * Initialize triangle vertices and call the drawing function
 */
void draw_triangle(GLFWwindow *win, const GLuint shad_prog) {
    // define vertices of the triangle in normalized device coordinates (NDC)
    static constexpr GLfloat vertices[] = {
       -0.5, -0.5, 0.0, // left
        0.5, -0.5, 0.0, // right
        0.0,  0.5, 0.0, // top
    };
    drawing_triangle(win, shad_prog, vertices, sizeof(vertices));
}

/*
 * Initialize triangles vertices and call the drawing function
 */
void draw_two_triangles(GLFWwindow *win, const GLuint shad_prog) {
    // define vertices of the triangle in normalized device coordinates (NDC)
    static constexpr GLfloat vertices[] = {
       -1.0f, -0.5f, 0.0f, // left
        0.0f, -0.5f, 0.0f, // right
       -0.5f,  0.5f, 0.0f, // top
        0.0f, -0.5f, 0.0f, // left
        1.0f, -0.5f, 0.0f, // right
        0.5f,  0.5f, 0.0f  // top
    };
    drawing_triangle(win, shad_prog, vertices, sizeof(vertices));
}

void draw_two_triangles2(GLFWwindow *win, const GLuint *shad_prog,
        const GLuint prog_size) {
    static constexpr GLfloat tri1[] = {
        -0.9f, -0.5f, 0.0f,  // Left
        -0.0f, -0.5f, 0.0f,  // Right
        -0.45f, 0.5f, 0.0f,  // Top
    };
    static constexpr GLfloat tri2[] = {
         0.0f, -0.5f, 0.0f,  // Left
         0.9f, -0.5f, 0.0f,  // Right
         0.45f, 0.5f, 0.0f   // Top
    };
    GLuint VBOs[2], VAOs[2];
    // We can also generate multiple VAOs or buffers at the same time
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);
    // First Triangle setup
    make_objects(VAOs[0], VBOs[0], tri1, sizeof(tri1));
    // Second Triangle setup
    make_objects(VAOs[1], VBOs[1], tri2, sizeof(tri2));

    // assuming that the triangles have the same size
    game_loop(win, VAOs, 2, shad_prog, prog_size, sizeof(tri1), 0);

    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
}

// draw two triangles with different shaders
void draw_two_triangles3(GLFWwindow *win) {
    /*
     * Build a shader program from two shaders: vertex and fragment.
     * The corresponding shaders are given as GLchar*
     */
    GLuint shad_prog[2];
    shad_prog[0] = shader_program(make_shader(GL_VERTEX_SHADER, vert_shad_src),
            make_shader(GL_FRAGMENT_SHADER, frag_shad_src));
    shad_prog[1] = shader_program(make_shader(GL_VERTEX_SHADER, vert_shad_src),
            make_shader(GL_FRAGMENT_SHADER, new_frag_shad_src));
    draw_two_triangles2(win, shad_prog, 2);
}

/*
 * Function for drawing an example triangle
 */
void drawing_triangle(GLFWwindow *win, const GLuint shad_prog,
        const GLfloat *vertices, const GLuint vert_size) {
    // generate and build objects
    GLuint VAO, VBO;
    gen_objects(&VAO, &VBO);
    make_objects(VAO, VBO, vertices, vert_size);

    // keep drawing
    game_loop(win, &VAO, 1, &shad_prog, 1, vert_size, 0);

    // cleaning up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

/*
 * Initialize rectangle vertices and indices, then call the drawing function
 */
void draw_rect(GLFWwindow *win, const GLuint shad_prog) {
    /*
     * Vertices of a rectangle made of two triangles
     */
    static constexpr GLfloat vertices[] = {
        0.5f,  0.5f, 0.0f, // top-right
        0.5f, -0.5f, 0.0f, // bottom-right
       -0.5f, -0.5f, 0.0f, // bottom-left
       -0.5f,  0.5f, 0.0f  // top-left
    };
    static constexpr GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    drawing_rect(win, shad_prog, vertices, sizeof(vertices),
            indices, sizeof(indices));
}

/*
 * Function for drawing an example rectangle
 */
void drawing_rect(GLFWwindow *win, const GLuint shad_prog,
        const GLfloat *vertices, const GLuint vert_size,
        const GLuint *indices, const GLuint ind_size) {
    // generate and build objects
    GLuint VAO, VBO, EBO;
    gen_objects(&VAO, &VBO, &EBO);
    make_objects(VAO, VBO, EBO, vertices, vert_size, indices, ind_size);

    // keep drawing
    game_loop(win, &VAO, 1, &shad_prog, 1, ind_size, 1);

    // cleaning up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

