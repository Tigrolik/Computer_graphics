/*
 * Following tutorial on OpenGL (colors):
 * http://learnopengl.com/#!Lighting/Colors
 *
 * Coloring a box as if illuminated by a lamp.
 * Practicing std::function a little bit in the main loop
 */

#include <iostream>
#include <vector>
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
// generate VAO, VBO, EBO...
void gen_objects(std::vector<GLuint>*, std::vector<GLuint>*);
// bind the objects with vertex data
void make_objects(const GLuint, const GLuint, const std::vector<GLfloat>&,
        const bool = true);

// drawing colored object and "lamp" box
void light_cube(GLFWwindow*);
void draw_light_objects(GLFWwindow*, const std::vector<GLfloat>&);
void light_obj_loop(GLFWwindow*, const std::vector<GLuint>&,
        const std::vector<Shader>&);
void draw_light_obj(const Shader&, const GLuint, const glm::mat4&,
        const glm::mat4&);
void draw_lamp(const Shader&, const GLuint, const glm::mat4&,
        const glm::mat4&);
void common_draw_light_obj(const Shader&, const GLuint, const glm::mat4&,
        const glm::mat4&, const int = 1);

// function to compute sizeof elements lying in the vector container
template <class T>
constexpr size_t size_of_elements(const std::vector<T> &v) {
    return v.size() * sizeof(T);
}

// here goes the main()
int main() try {

    static constexpr GLuint width {800}, height {600};
    last_x = width >> 1;
    last_y = height >> 1;
    GLFWwindow *win = init(width, height);

    std::cout << "This program is simply a demonstration of two objects:\n" <<
        "lamp (white cube) and illuminated object (colored cube)\n";

    // draw the colored cube and "lamp"
    light_cube(win);

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
    GLFWwindow *win = glfwCreateWindow(w, h, "Lighting", nullptr, nullptr);
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

    // enable depth testing for nice 3D output
    glEnable(GL_DEPTH_TEST);

    return win;
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

    const GLfloat xoffset = xpos - last_x, yoffset = last_y - ypos;

    last_x = xpos;
    last_y = ypos;

    main_cam.process_mouse_move(xoffset, yoffset);
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
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        main_cam.process_keyboard(Camera::forward_dir, delta_frame_time);
    else if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        main_cam.process_keyboard(Camera::backward_dir, delta_frame_time);
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

/*
 * Generate objects like Vertex Array Objects, Vertex Buffer Object using
 * pointer to std::vector (trying to achieve more general solution)
 */
void gen_objects(std::vector<GLuint> *VAO_vec, std::vector<GLuint> *VBO_vec) {
    for (size_t i {0}; i < (*VAO_vec).size(); ++i)
        glGenVertexArrays(1, &(*VAO_vec)[i]);
    for (size_t i {0}; i < (*VBO_vec).size(); ++i)
        glGenBuffers(1, &(*VBO_vec)[i]);
}

/*
 * Binding objects with vertex data.
 */
void make_objects(const GLuint VAO, const GLuint VBO,
        const std::vector<GLfloat> &vertices, const bool fill_VBO) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (fill_VBO)
        glBufferData(GL_ARRAY_BUFFER, size_of_elements(vertices),
                vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
            (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

// draw a colored box together with a "lamp" box
void light_cube(GLFWwindow *win) {
    // define vertices and indices for the cube
    static const std::vector<GLfloat> vertices {
       -0.5, -0.5, -0.5,
        0.5, -0.5, -0.5,
        0.5,  0.5, -0.5,
        0.5,  0.5, -0.5,
       -0.5,  0.5, -0.5,
       -0.5, -0.5, -0.5,

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
        0.5,  0.5, -0.5,
        0.5, -0.5, -0.5,
        0.5, -0.5, -0.5,
        0.5, -0.5,  0.5,
        0.5,  0.5,  0.5,

       -0.5, -0.5, -0.5,
        0.5, -0.5, -0.5,
        0.5, -0.5,  0.5,
        0.5, -0.5,  0.5,
       -0.5, -0.5,  0.5,
       -0.5, -0.5, -0.5,

       -0.5,  0.5, -0.5,
        0.5,  0.5, -0.5,
        0.5,  0.5,  0.5,
        0.5,  0.5,  0.5,
       -0.5,  0.5,  0.5,
       -0.5,  0.5, -0.5,
    };

    draw_light_objects(win, vertices);
}

// helper function to draw lighting objects
void draw_light_objects(GLFWwindow *win, const std::vector<GLfloat> &vertices) {
    const Shader light_shader {shad_path + "light_shader_01.vs",
        shad_path + "light_shader_01.frag"};
    const Shader lamp_shader {shad_path + "lamp_shader_01.vs",
        shad_path + "lamp_shader_01.frag"};

    GLuint VAO_obj {}, VAO_lamp {}, VBO {};

    std::vector<GLuint> VAO_vec {VAO_obj, VAO_lamp};
    std::vector<GLuint> VBO_vec {VBO};

    gen_objects(&VAO_vec, &VBO_vec);
    make_objects(VAO_vec[0], VBO_vec[0], vertices, true);
    make_objects(VAO_vec[1], VBO_vec[0], vertices, false);

    light_obj_loop(win, VAO_vec,
            std::vector<Shader> {light_shader, lamp_shader});
}

// main loop for drawing light objects
void light_obj_loop(GLFWwindow *win,
        const std::vector<GLuint> &VAO, const std::vector<Shader> &shad) {
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    const auto win_asp = float(win_w) / win_h;
    // practicing std::function a little bit
    //const std::vector<std::function<void(const Shader&, const GLuint&,
    //        const glm::mat4&, const glm::mat4&)>> draw_funs = {draw_light_obj,
    //    draw_lamp};
    std::function<void(const Shader&, const GLuint&, const glm::mat4&,
            const glm::mat4&, const int)> draw_fun = common_draw_light_obj;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;
        do_movement();
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const auto view = main_cam.view_matrix();
        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);

        for (int i = 0; i < 2; ++i)
            draw_fun(shad[i], VAO[i], view, proj, i + 1);
        //draw_funs[0](shad[0], VAO[0], view, proj);
        //draw_funs[1](shad[1], VAO[1], view, proj);

        glfwSwapBuffers(win);
    }
}

// helper function for drawing colored object
void draw_light_obj(const Shader &shad, const GLuint VAO, const glm::mat4 &view,
        const glm::mat4 &proj) {
    shad.use();
    const auto model_loc = glGetUniformLocation(shad.id(), "model");
    const auto view_loc  = glGetUniformLocation(shad.id(), "view");
    const auto proj_loc  = glGetUniformLocation(shad.id(), "proj");
    glBindVertexArray(VAO);

    const GLint obj_color_loc = glGetUniformLocation(shad.id(),
            "object_color");
    const GLint light_color_loc = glGetUniformLocation(shad.id(),
            "light_color");
    glUniform3f(obj_color_loc, 1, 0.5, 0.31);
    glUniform3f(light_color_loc, 1, 0.75, 1);

    glm::mat4 model;
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// helper function for drawing "lamp" object
void draw_lamp(const Shader &shad, const GLuint VAO, const glm::mat4 &view,
        const glm::mat4 &proj) {
    shad.use();
    const auto model_loc = glGetUniformLocation(shad.id(), "model");
    const auto view_loc  = glGetUniformLocation(shad.id(), "view");
    const auto proj_loc  = glGetUniformLocation(shad.id(), "proj");
    glBindVertexArray(VAO);

    static const glm::vec3 lamp_pos {1.2, 1, 2};
    glm::mat4 model = glm::translate(glm::mat4{}, lamp_pos);
    model = glm::scale(model, glm::vec3{0.2});

    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// common helper function for drawing a light object (used for std::function)
void common_draw_light_obj(const Shader &shad, const GLuint VAO,
        const glm::mat4 &view, const glm::mat4 &proj, const int option) {
    if (option == 1)
        draw_light_obj(shad, VAO, view, proj);
    else
        draw_lamp(shad, VAO, view, proj);
}


