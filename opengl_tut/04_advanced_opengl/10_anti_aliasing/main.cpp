/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing
 *
 * Demonstration of anti aliasing techniques on a cube object
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
#include "../../classes/Model.h"

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
static Camera main_cam {glm::vec3{0, 0, 3}};

/*
 * Functions declarations
 */
// initialize stuff
GLFWwindow* init(const GLuint, const GLuint, const int = 0);
// callback functions
void key_callback(GLFWwindow*, const int, const int, const int, const int);
void mouse_callback(GLFWwindow*, const double, const double);
void scroll_callback(GLFWwindow*, const double, const double);
// movement function
void do_movement();
// cleaning up
int clean_up(const int);

// generate VAO and VBO for cube vertices
void gen_cube_vao_vbo(GLuint&, GLuint&, const std::vector<GLfloat>&,
        const GLuint, const GLuint, const bool = false);
// cube vertices
std::vector<GLfloat> cube_vertices();
// quad vertices
std::vector<GLfloat> quad_vertices();

// generate framebuffer id
GLuint gen_framebuffer();
// Create framebuffer
GLuint make_framebuffer_ms(const int, const int, const GLuint num_samples);
GLuint make_framebuffer_ms(GLFWwindow*, const GLuint num_samples);
// Create a renderbuffer
GLuint make_renderbuffer_ms(const int, const int);
GLuint make_renderbuffer_ms(GLFWwindow*);
// generate texture for the framebuffer
GLuint make_texture_fb(const int, const int);
GLuint make_texture_fb(GLFWwindow*);

// draw object
void draw_object(const Shader&, const GLuint, const GLuint, const GLuint,
        const int, const int, const GLuint);
// draw quad
void draw_quad(const Shader&, const GLuint, const GLuint);
// drawing loop
void game_loop(GLFWwindow*, const int = 0);

// function to compute aspect ratio of screen's width and height
float window_aspect_ratio(GLFWwindow*);
// function to compute sizeof elements lying in the vector container
template <class T>
constexpr size_t size_in_bytes(const std::vector<T>& v) {
    return v.size() * sizeof(T);
}

// process user input
void process_input(const std::string&, const int, const int);
// display menu of possible actions and process them
void show_menu(const std::string&, const int, const int);

// here goes the main()
int main(int argc, char *argv[]) try {

    static constexpr GLuint width {800}, height {600};
    last_x = width >> 1;
    last_y = height >> 1;

    std::cout <<
        "----------------------------------------------------------------\n" <<
        "This program demonstrates the use of MSAA tecnique:\n" <<
        "keys A/D, left/right arrow keys control side camera movement\n" <<
        "up/down arrow keys - up and down, W/S - depth\n" <<
        "mouse can also be used to change view/zoom (scroll)\n" <<
        "----------------------------------------------------------------\n";

    if (argc > 1)
        process_input(argv[1], width, height);
    else
        show_menu(argv[0], width, height);

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
 * Process user input
 */
void process_input(const std::string &inp, const int win_w, const int win_h) {
    static constexpr char num_options {'3'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char < num_options) {
        const int option = inp_char - '0';
        GLFWwindow *win = init(win_w, win_h, option);
        game_loop(win, option);
    } else {
        std::cerr << "Wrong input: drawing default scene\n";
        GLFWwindow *win = init(win_w, win_h);
        game_loop(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(const std::string &prog_name, const int win_w, const int win_h) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tgreen cube with GLFW MSAA (default)\n" <<
        "1:\tgreen cube with renderbuffer object MSAA\n" <<
        "2:\t\"blurred\" cube\n";
    GLFWwindow *win = init(win_w, win_h);
    game_loop(win, 0);
}

/*
 * Function to initialize stuff. With comparison to the first tutorial here we
 * put more stuff inside a single function, though trying not to make it too
 * long (keeping the idea of fitting a single function on a screen in mind)
 */
GLFWwindow* init(const GLuint w, const GLuint h, const int option) {
    glfwInit();

    // configuring GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // hint GLFW about using multisampling with N samples
    if (option == 0)
        glfwWindowHint(GLFW_SAMPLES, 4);

    // create a window object
    GLFWwindow *win = glfwCreateWindow(w, h, "Anti Aliasing", nullptr, nullptr);
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

    // enable multisampling
    glEnable(GL_MULTISAMPLE);

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
void gen_cube_vao_vbo(GLuint& VAO, GLuint& VBO, const std::vector<GLfloat>& v,
        const GLuint stride, const GLuint offset, const bool second_attr) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(v), v.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, offset, GL_FLOAT, GL_FALSE,
            stride * sizeof(GLfloat), (GLvoid*)0);

    if (second_attr) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, offset, GL_FLOAT, GL_FALSE,
                stride * sizeof(GLfloat), (GLvoid*)(offset * sizeof(GLfloat)));
    }

    glBindVertexArray(0);
}

// cube vertices
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

// quad vertices
std::vector<GLfloat> quad_vertices() {
    return std::vector<GLfloat> {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
}

// generate framebuffer id
GLuint gen_framebuffer() {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    // bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    return fbo;
}

// create framebuffer
GLuint make_framebuffer_ms(const int win_w, const int win_h,
        const GLuint num_samples) {
    // create a framebuffer object
    const GLuint fbo {gen_framebuffer()};

    // create a multisampled color attachment texture
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_id);

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGB,
            win_w, win_h, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D_MULTISAMPLE, tex_id, 0);

    return fbo;
}

// version with GLFWwindow as a parameter
GLuint make_framebuffer_ms(GLFWwindow *win, const GLuint num_samples) {
    // get window size
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    return make_framebuffer_ms(win_w, win_h, num_samples);
}

// Create a renderbuffer
GLuint make_renderbuffer_ms(const int win_w, const int win_h) {
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
            win_w, win_h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach the renderbuffer object to the depth and stencil attachment
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, rbo);

    // check the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error {"Framebuffer is not complete"};
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return rbo;
}

// version with GLFWwindow as a parameter
GLuint make_renderbuffer_ms(GLFWwindow *win) {
    // get window size
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    return make_renderbuffer_ms(win_w, win_h);
}

// generate texture for the framebuffer
GLuint make_texture_fb(const int win_w, const int win_h) {
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win_w, win_h, 0, GL_RGB,
            GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach texture to the currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            tex_id, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error {"Framebuffer is not complete"};
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return tex_id;
}

// version with GLFWwindow as a parameter
GLuint make_texture_fb(GLFWwindow *win) {
    // get window size
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);

    return make_texture_fb(win_w, win_h);
}

// main drawing loop
void game_loop(GLFWwindow* win, const int option) {
    const Shader shad {shad_path + "lamp_shader_01.vs",
        shad_path + "ubo_green_01.frag"};

    GLuint cube_vao {}, cube_vbo {}, quad_vao {}, quad_vbo {};
    gen_cube_vao_vbo(cube_vao, cube_vbo, cube_vertices(), 3, 3);
    gen_cube_vao_vbo(quad_vao, quad_vbo, quad_vertices(), 4, 2, true);

    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    const GLuint fbo = make_framebuffer_ms(win_w, win_h, 4);
    make_renderbuffer_ms(win);

    const GLuint fbo2 = gen_framebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    const GLuint tex_id = make_texture_fb(win_w, win_h);

    const Shader quad_shad {shad_path + "framebuffer_01.vs",
        shad_path + "framebuffer_04.frag"};
    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;
        glfwPollEvents();
        do_movement();

        draw_object(shad, cube_vao, fbo, fbo2, win_w, win_h, option);
        if (option == 2)
            draw_quad(quad_shad, quad_vao, tex_id);

        glfwSwapBuffers(win);
    }
}

// draw object
void draw_object(const Shader& shad, const GLuint vao, const GLuint fbo,
        const GLuint fbo2, const int win_w, const int win_h,
        const GLuint option) {
    // draw scene in multisampled buffers
    if (option > 0)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.2, 0.2, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    const auto idx = shad.id();
    shad.use();
    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(glm::perspective(main_cam.zoom(),
                    float(win_w) / win_h, 0.1f, 1000.0f)));
    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(main_cam.view_matrix()));
    glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
            glm::value_ptr(glm::mat4{}));

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    if (option > 0) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        if (option == 2)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);
        else
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, win_w, win_h, 0, 0, win_w, win_h,
                GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
}

// draw quad
void draw_quad(const Shader& shad, const GLuint vao, const GLuint tex_id) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.2, 0.2, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    shad.use();
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

