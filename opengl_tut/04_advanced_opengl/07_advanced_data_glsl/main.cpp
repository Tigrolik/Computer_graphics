/*
 * Following tutorial on OpenGL (materials):
 * http://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
 *
 * Demonstration of using of uniform buffer objects
 *
 */

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <cassert>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

// GL math
//#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../classes/Shader.h"
#include "../../classes/Camera.h"

// paths to the folder where we keep shaders and textures: global vars
static const std::string shad_path {"../../shaders/"};
static const std::string tex_path {"../../images/"};
static const std::string model_path {"../../models/"};

// tracking which keys have been pressed/released (for smooth movement)
static bool keys[1024];

// global values to keep track of time between the frames
static GLfloat delta_frame_time = 0;
static GLfloat last_frame_time  = 0;

// last cursor position
static GLfloat last_x = 0, last_y = 0;

// avoid sudden jump of the camera at the beginning
static bool first_mouse_move = true;

// using Camera class
static Camera main_cam {glm::vec3{0, 0, 5}};

/*
 * Functions declarations
 */
// initialize stuff
GLFWwindow* init(const GLuint, const GLuint);
// callback functions
void key_callback(GLFWwindow*, const int, const int, const int, const int);
void mouse_callback(GLFWwindow*, const double, const double);
void scroll_callback(GLFWwindow*, const double, const double);
// movement function
void do_movement();
// cleaning up
int clean_up(const int);

// generate VAO and VBO for a cube
void gen_cube_vao_vbo(GLuint&, GLuint&, const std::vector<GLfloat>&);
// create a uniform buffer object
void gen_shader_ubo(const Shader&, const std::string&);
// generate a uniform buffer object
GLuint gen_uniform_buffer(const GLuint);
// set a glm::mat4 matrix into the uniform block
void store_mat4_to_ubo(const glm::mat4&, const GLuint, const GLuint);

// retrieve vertices for a cube
std::vector<GLfloat> cube_vertices();

// drawing objects
void cube_test(GLFWwindow*);
void game_loop(GLFWwindow*, const GLuint);
void draw_cube(const Shader&, const glm::mat4&);

// function to compute aspect ratio of screen's width and height
float window_aspect_ratio(GLFWwindow*);
// function to compute sizeof elements lying in the vector container
template <class T>
constexpr size_t size_in_bytes(const std::vector<T>& v) {
    return v.size() * sizeof(T);
}

// here goes the main()
int main() try {

    static constexpr GLuint width {800}, height {600};
    last_x = width >> 1;
    last_y = height >> 1;
    GLFWwindow *win = init(width, height);

    std::cout <<
        "----------------------------------------------------------------\n" <<
        "This program demonstrates four cubes drawn with the use of uniform" <<
        " buffer objects:\n" <<
        "keys A/D, left/right arrow keys control side camera movement\n" <<
        "up/down arrow keys - up and down, W/S - depth\n" <<
        "mouse can also be used to change view/zoom (scroll)\n" <<
        "----------------------------------------------------------------\n";

    cube_test(win);

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

    // configuring GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // create a window object
    GLFWwindow *win = glfwCreateWindow(w, h, "Advanced data and GLSL",
            nullptr, nullptr);
    if (win == nullptr)
        throw std::runtime_error {"Failed to create GLFW window"};

    glfwMakeContextCurrent(win);

    // let GLEW know to use a modern approach to retrieving function pointers
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error {"Failed to initialize GLEW"};

    // register callbacks
    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, mouse_callback);
    glfwSetScrollCallback(win, scroll_callback);

    // disable cursor
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // inform OpenGL about the size of the rendering window
    glViewport(0, 0, w, h);

    // enable the depth test
    glEnable(GL_DEPTH_TEST);

    return win;
}

// function to calculate aspect ratio of screen's width and height
float window_aspect_ratio(GLFWwindow *win) {
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    return float(win_w) / win_h;
}

/*
 * Call this function whenever a key is pressed / released
 */
void key_callback(GLFWwindow *win, const int key, const int,
        const int action, const int) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
            glfwSetWindowShouldClose(win, GL_TRUE);
        else
            keys[key] = true;
    } else if (action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

/*
 * Call this function whenever pointer (mouse) moves
 */
void mouse_callback(GLFWwindow*, const double xpos, const double ypos) {
    if (first_mouse_move) {
        last_x = xpos;
        last_y = ypos;
        first_mouse_move = false;
    }

    main_cam.process_mouse_move(xpos - last_x, last_y - ypos);

    last_x = xpos;
    last_y = ypos;
}

/*
 * Call this function during scrolling
 */
void scroll_callback(GLFWwindow*, const double, const double yoffset) {
    main_cam.process_scroll(yoffset);
}

/*
 * Function for smooth movement of the camera
 */
void do_movement() {
    if (keys[GLFW_KEY_UP])
        main_cam.process_keyboard(Camera::up_dir, delta_frame_time);
    else if (keys[GLFW_KEY_DOWN])
        main_cam.process_keyboard(Camera::down_dir, delta_frame_time);
    else if (keys[GLFW_KEY_S])
        main_cam.process_keyboard(Camera::backward_dir, delta_frame_time);
    else if (keys[GLFW_KEY_W])
        main_cam.process_keyboard(Camera::forward_dir, delta_frame_time);
    else if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        main_cam.process_keyboard(Camera::left_dir, delta_frame_time);
    else if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        main_cam.process_keyboard(Camera::right_dir, delta_frame_time);
}

/*
 * Properly clean up and return the value as an indicator of success (0) or
 * failure (1 or other non-zero value)
 */
int clean_up(const int val) {
    glfwTerminate();
    return val;
}

// generate VAO and VBO for a cube object (presented by its vertices)
void gen_cube_vao_vbo(GLuint& VAO, GLuint& VBO, const std::vector<GLfloat>& v) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(v), v.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
            (GLvoid*)0);

    glBindVertexArray(0);
}

// create a uniform buffer object
void gen_shader_ubo(const Shader& shad, const std::string& str) {
    const auto idx = shad.id();
    //const GLuint ubo_idx {glGetUniformBlockIndex(idx, str.c_str())};
    glUniformBlockBinding(idx, glGetUniformBlockIndex(idx, str.c_str()), 0);
}

// generate the buffer
GLuint gen_uniform_buffer(const GLuint buf_size) {
    GLuint UBO;
    glGenBuffers(1, &UBO);

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, buf_size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // define the range of the buffer: links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, buf_size);

    return UBO;
}

// set a glm::mat4 matrix into the uniform block
void store_mat4_to_ubo(const glm::mat4& m, const GLuint offset_val,
        const GLuint UBO) {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, offset_val, sizeof(glm::mat4),
            glm::value_ptr(m));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// get cube object vertices with normal vectors
std::vector<GLfloat> cube_vertices() {
    return std::vector<GLfloat> {
        -0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
        -0.5, -0.5, -0.5,
        -0.5,  0.5, -0.5,

        -0.5, -0.5,  0.5,
         0.5, -0.5,  0.5,
         0.5,  0.5,  0.5,
         0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,
        -0.5, -0.5,  0.5,

        -0.5,  0.5,  0.5,
        -0.5,  0.5, -0.5,
        -0.5, -0.5, -0.5,
        -0.5, -0.5, -0.5,
        -0.5, -0.5,  0.5,
        -0.5,  0.5,  0.5,

         0.5,  0.5,  0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5,  0.5,
         0.5, -0.5,  0.5,

        -0.5, -0.5, -0.5,
         0.5, -0.5, -0.5,
         0.5, -0.5,  0.5,
         0.5, -0.5,  0.5,
        -0.5, -0.5,  0.5,
        -0.5, -0.5, -0.5,

        -0.5,  0.5, -0.5,
         0.5,  0.5,  0.5,
         0.5,  0.5, -0.5,
         0.5,  0.5,  0.5,
        -0.5,  0.5, -0.5,
        -0.5,  0.5,  0.5
    };
}

// main testing function: drawing cubes
void cube_test(GLFWwindow *win) {
    static const std::vector<GLfloat> cube_verts {cube_vertices()};

    // setup VAO and VBO for our cubes
    GLuint cube_vao{}, cube_vbo{};
    gen_cube_vao_vbo(cube_vao, cube_vbo, cube_verts);

    game_loop(win, cube_vao);
}

// main loop function
void game_loop(GLFWwindow* win, const GLuint VAO) {
    static const std::string vtx_path {shad_path + "advanced_glsl_01.vs"};
    static const std::vector<Shader> shads {
        Shader {vtx_path, shad_path + "ubo_red_01.frag"},
        Shader {vtx_path, shad_path + "ubo_green_01.frag"},
        Shader {vtx_path, shad_path + "ubo_blue_01.frag"},
        Shader {vtx_path, shad_path + "ubo_yellow_01.frag"}};
    for (const auto &s: shads)
        gen_shader_ubo(s, "Matrices");

    static const std::vector<glm::vec3> cubes_pos {{-0.75, 0.75, 0},
        {0.75, 0.75, 0}, {-0.75, -0.75, 0}, {0.75, -0.75,0}};
    if (shads.size() != cubes_pos.size())
        throw std::runtime_error {"number of shaders and cubes mismatch"};

    static const GLuint buf_size {sizeof(glm::mat4)};
    const GLuint UBO {gen_uniform_buffer(buf_size << 1)};

    // store the projection matrix in the ubo
    const auto win_asp = window_aspect_ratio(win);
    store_mat4_to_ubo(glm::perspective(45.0f, win_asp, 0.1f, 100.0f), 0, UBO);


    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;

        glfwPollEvents();
        do_movement();

        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // store the view matrix in the ubo
        store_mat4_to_ubo(main_cam.view_matrix(), buf_size, UBO);

        // draw our cubes
        glBindVertexArray(VAO);
        for (GLuint i {0}; i < shads.size(); ++i)
            draw_cube(shads[i], glm::translate(glm::mat4{}, cubes_pos[i]));
        glBindVertexArray(0);

        glfwSwapBuffers(win);
    }
}

// drawing a cube
void draw_cube(const Shader& shad, const glm::mat4& mod) {
    shad.use();
    glUniformMatrix4fv(glGetUniformLocation(shad.id(), "model"), 1, GL_FALSE,
            glm::value_ptr(mod));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

