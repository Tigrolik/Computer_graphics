/*
 * Following tutorial on OpenGL:
 * http://learnopengl.com/#!Getting-started/Shaders
 *
 * Now we created a Shader class:
 * reading vertex and fragment shaders from files
 *
 * The choice of extensions for shaders have fallen to .vs and .frag
 */

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"

// path to the folder where we keep shaders
static const std::string shad_path {"../../shaders/"};

/*
 * Functions declarations
 */
// initialize stuff
GLFWwindow* init(const GLuint, const GLuint);
// callback function
void key_callback(GLFWwindow*, const int, const int, const int, const int);
// drawing loop
void game_loop(GLFWwindow*, const GLuint, const Shader&);
// cleaning up
int clean_up(const int);
// generate VAO, VBO, EBO...
void gen_objects(GLuint*, GLuint*);
// bind the objects with vertex data
void make_objects(const GLuint, const GLuint, const GLfloat*, const GLuint);

// --- functions for drawing triangles ---
void draw_red_triangle(GLFWwindow*);
void draw_up_down_triangle(GLFWwindow*);
void draw_offset_triangle(GLFWwindow*);
void draw_pos2col_triangle(GLFWwindow*);
void draw_glow_triangle(GLFWwindow*);
void draw_palette_triangle(GLFWwindow*);
void drawing_triangle(GLFWwindow*, const Shader&, const GLfloat*, const GLuint);

// here goes the main()
int main(int argc, char *argv[]) try {
    static constexpr GLuint width {800}, height {600};
    GLFWwindow *win = init(width, height);
    static constexpr char num_options {'5'};
    if (argc > 1) {
        const std::string s {argv[1]};
        const char inp_char {s[0]};
        if (s.length() == 1 && inp_char >= '0' && inp_char <= num_options) {
            switch (inp_char - '0') {
                case 1:
                    draw_glow_triangle(win);
                    break;
                case 2:
                    draw_up_down_triangle(win);
                    break;
                case 3:
                    draw_offset_triangle(win);
                    break;
                case 4:
                    draw_pos2col_triangle(win);
                    break;
                case 5:
                    draw_palette_triangle(win);
                    break;
                case 0:
                default:
                    draw_red_triangle(win);
            }
        } else {
            std::cerr << "Wrong input: drawing default triangle\n";
            draw_red_triangle(win);
        }
    } else {
        std::cout << "Note: the program can be used as follows:\n" <<
            argv[0] << " int_param, where int_param can be:\n" <<
            "0:\tred triangle (default)\n" << "1:\tglowing triangle\n" <<
            "2:\ttriangle upside down\n3:\toffset triangle\n" <<
            "4:\ttriangle (position to color)\n5:\tpalette triangle\n";
        draw_red_triangle(win);
    }
    // clean up and exit properly
    return clean_up(0);
} catch (const std::runtime_error &e) {
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
        throw std::runtime_error {"Failed to create GLFW window"};

    glfwMakeContextCurrent(win);

    // let GLEW know to use a modern approach to retrieving function pointers
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error {"Failed to initialize GLEW"};

    // register the callback
    glfwSetKeyCallback(win, key_callback);

    // inform OpenGL about the size of the rendering window
    glViewport(0, 0, w, h);

    return win;
}

/*
 * Game loop - keep things drawing
 */
void game_loop(GLFWwindow *win, const GLuint VAO, const Shader &shad) {
    // polygon drawing mode: GL_FILL or GL_LINE
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // check if GLFW has been told to close
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        // --- Rendering happens here ---
        // Set the color, clear the color buffer
        glClearColor(0.33f, 0.44f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // drawing
        shad.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
 * Generate objects like Vertex Array Objects, Vertex Buffer Object
 */
void gen_objects(GLuint *VAO, GLuint *VBO) {
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
}

/*
 * Bind objects with vertex data.
 */
void make_objects(const GLuint VAO, const GLuint VBO,
        const GLfloat *vertices, const GLuint vert_size) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vert_size, vertices, GL_STATIC_DRAW);
    /*
     * Here we use some crutches for managing different vertices, but
     * that suffices for these simple examples
     */
    if (vert_size / sizeof(GLfloat) < 10) {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                (GLvoid*)0);
        glEnableVertexAttribArray(0);
    } else {
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                (GLvoid *)0);
        glEnableVertexAttribArray(0);
        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                (GLvoid *)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// init vertices, shader and call the drawing function
void draw_red_triangle( GLFWwindow *win) {
    // define vertices of the triangle in normalized device coordinates (NDC)
    static constexpr GLfloat vertices[] = {
       -0.5, -0.5, 0.0, // left
        0.5, -0.5, 0.0, // right
        0.0,  0.5, 0.0, // top
    };
    const Shader shad {shad_path + "red_triangle.vs",
        shad_path + "red_triangle.frag"};
    drawing_triangle(win, shad, vertices, sizeof(vertices));
}

void draw_glow_triangle(GLFWwindow *win) {
    // define vertices of the triangle in normalized device coordinates (NDC)
    static constexpr GLfloat vertices[] = {
       -0.5, -0.5, 0.0, // left
        0.5, -0.5, 0.0, // right
        0.0,  0.5, 0.0, // top
    };

    // generate and build objects
    GLuint VAO, VBO;
    gen_objects(&VAO, &VBO);
    make_objects(VAO, VBO, vertices, sizeof(vertices));
    const Shader shad {shad_path + "glow_triangle.vs",
        shad_path + "glow_triangle.frag"};

    // keep drawing
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents(); // check for triggered events
        glClearColor(0.33, 0.44, 0.85, 1);
        //glClearColor(0.5 + sin(glfwGetTime() * 3) * 0.5,
        //        0.5 + sin(glfwGetTime() * 5) * 0.5,
        //        0.5 + sin(glfwGetTime() * 4) * 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        shad.use();

        // set the uniform color
        const GLfloat time_val = glfwGetTime();
        const GLfloat green_val = 0.5 + sin(time_val) * 0.5;
        const GLint color_loc = glGetUniformLocation(shad.id(), "color_val");
        // experimenting with the colors
        glUniform4f(color_loc, 0.5 + sin(time_val * 7) * 0.5, green_val,
                0.5 + sin(time_val * 2) * 0.5, 0.2);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // swap the screen buffers
        glfwSwapBuffers(win);
    }

    // cleaning up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void draw_up_down_triangle(GLFWwindow *win) {
    static constexpr GLfloat vertices[] = {
       -0.5, -0.5, 0.0, // left
        0.5, -0.5, 0.0, // right
        0.0,  0.5, 0.0, // top
    };

    const Shader shad {shad_path + "up_down_triangle.vs",
        shad_path + "up_down_triangle.frag"};
    drawing_triangle(win, shad, vertices, sizeof(vertices));
}

void draw_offset_triangle(GLFWwindow *win) {
    // define vertices of the triangle in normalized device coordinates (NDC)
    static constexpr GLfloat vertices[] = {
       -0.5, -0.5, 0.0, // left
        0.5, -0.5, 0.0, // right
        0.0,  0.5, 0.0, // top
    };

    // generate and build objects
    GLuint VAO, VBO;
    gen_objects(&VAO, &VBO);
    make_objects(VAO, VBO, vertices, sizeof(vertices));

    const Shader shad {shad_path + "offset_triangle.vs",
        shad_path + "offset_triangle.frag"};
    // define an offset
    const GLfloat x_off {0.5};

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents(); // check for triggered events
        glClearColor(0.33f, 0.44f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shad.use();

        // set the uniform offset for x
        glUniform1f(glGetUniformLocation(shad.id(), "x_offset"), x_off);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(win);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void draw_pos2col_triangle(GLFWwindow *win) {
    static constexpr GLfloat vertices[] = {
       -0.5, -0.5, 0.0, // left
        0.5, -0.5, 0.0, // right
        0.0,  0.5, 0.0, // top
    };

    const Shader shad {shad_path + "pos_to_color.vs",
        shad_path + "pos_to_color.frag"};
    drawing_triangle(win, shad, vertices, sizeof(vertices));
}

void draw_palette_triangle(GLFWwindow *win) {
    static constexpr GLfloat vertices[] = {
        // positions         // colors
        0.5, -0.5, 0, 1, 0, 0,  // bottom right
       -0.5, -0.5, 0, 0, 1, 0,  // bottom left
        0.0,  0.5, 0, 0, 0, 1   // top
    };
    const Shader shad {shad_path + "palette_triangle.vs",
        shad_path + "palette_triangle.frag"};
    drawing_triangle(win, shad, vertices, sizeof(vertices));
}

void drawing_triangle(GLFWwindow *win, const Shader &shad,
        const GLfloat *vertices, const GLuint vert_size) {
    // generate and build objects
    GLuint VAO, VBO;
    gen_objects(&VAO, &VBO);
    make_objects(VAO, VBO, vertices, vert_size);

    // keep drawing
    game_loop(win, VAO, shad);

    // cleaning up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

