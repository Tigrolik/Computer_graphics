/*
 * Following tutorial on OpenGL (materials):
 * http://learnopengl.com/#!Lighting/Materials
 *
 * Demonstrating how materials can be expressed via colors
 *
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

#include "Shader.h"
#include "Camera.h"

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
        const bool, const int);

// drawing colored object and "lamp" box with various lighting
void diffuse_light_cube(GLFWwindow*, const int = 0);
void draw_objects(GLFWwindow*, const std::vector<GLfloat>&, const int = 0,
        const bool = false);
void light_obj_loop(GLFWwindow*, const std::vector<GLuint>&,
        const std::vector<Shader>&, const bool, const int);
void draw_light_obj(const Shader&, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::vec3&, const int);
void draw_lamp(const Shader&, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::vec3&);
void common_draw_light_obj(const Shader&, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::vec3&, const int, const int);

// function to compute sizeof elements lying in the vector container
template <class T>
constexpr size_t size_of_elements(const std::vector<T> &v) {
    return v.size() * sizeof(T);
}

// process user input
void process_input(GLFWwindow*, const std::string&);
// display menu of possible actions and process them
void show_menu(GLFWwindow*, const std::string&);

// here goes the main()
int main(int argc, char *argv[]) try {

    static constexpr GLuint width {800}, height {600};
    last_x = width >> 1;
    last_y = height >> 1;
    GLFWwindow *win = init(width, height);

    std::cout <<
        "----------------------------------------------------------------\n" <<
        "This program demonstrates how material properties can be simulated" <<
        " with colors. The scene contains two objects:\n" <<
        "lamp (white cube) and illuminated object (colored cube)\n" <<
        "keys A/D, left/right arrow keys control side camera movement\n" <<
        "keys W/S - up and down, arrows up/down - depth\n" <<
        "mouse can also be used to change view/zoom (scroll)\n" <<
        "----------------------------------------------------------------\n";

    if (argc > 1)
        process_input(win, argv[1]);
    else
        show_menu(win, argv[0]);

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
void process_input(GLFWwindow *win, const std::string &inp) {
    static constexpr char num_options {'3'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char < num_options) {
        switch (inp_char - '0') {
            case 2:
                diffuse_light_cube(win, 2);
                break;
            case 1:
                diffuse_light_cube(win, 1);
                break;
            case 0:
            default:
                diffuse_light_cube(win, 0);
        }
    } else {
        std::cerr << "Wrong input: drawing default cube\n";
        diffuse_light_cube(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tcube with \"bronze\" material (default)\n" <<
        "1:\tcube with lighting (\"material\") changing with time\n" <<
        "2:\tcube with cyan plastic lighting\n";
    diffuse_light_cube(win, 0);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Materials", nullptr,
            nullptr);
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
    if (keys[GLFW_KEY_UP])
        main_cam.process_keyboard(Camera::forward_dir, delta_frame_time);
    else if (keys[GLFW_KEY_DOWN])
        main_cam.process_keyboard(Camera::backward_dir, delta_frame_time);
    else if (keys[GLFW_KEY_S])
        main_cam.process_keyboard(Camera::down_dir, delta_frame_time);
    else if (keys[GLFW_KEY_W])
        main_cam.process_keyboard(Camera::up_dir, delta_frame_time);
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
        const std::vector<GLfloat> &vertices, const bool fill_VBO,
        const int option) {

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (fill_VBO)
        glBufferData(GL_ARRAY_BUFFER, size_of_elements(vertices),
                vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (vertices.size() / 36) *
            sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    if (option == 1) {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (vertices.size() / 36) *
                sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }


    glBindVertexArray(0);
}

// draw a box enlighted by a diffused light
void diffuse_light_cube(GLFWwindow *win, const int option) {
    static const std::vector<GLfloat> vertices {
        -0.5, -0.5, -0.5,  0,  0, -1,
         0.5, -0.5, -0.5,  0,  0, -1,
         0.5,  0.5, -0.5,  0,  0, -1,
         0.5,  0.5, -0.5,  0,  0, -1,
        -0.5,  0.5, -0.5,  0,  0, -1,
        -0.5, -0.5, -0.5,  0,  0, -1,

        -0.5, -0.5,  0.5,  0,  0,  1,
         0.5, -0.5,  0.5,  0,  0,  1,
         0.5,  0.5,  0.5,  0,  0,  1,
         0.5,  0.5,  0.5,  0,  0,  1,
        -0.5,  0.5,  0.5,  0,  0,  1,
        -0.5, -0.5,  0.5,  0,  0,  1,

        -0.5,  0.5,  0.5, -1,  0,  0,
        -0.5,  0.5, -0.5, -1,  0,  0,
        -0.5, -0.5, -0.5, -1,  0,  0,
        -0.5, -0.5, -0.5, -1,  0,  0,
        -0.5, -0.5,  0.5, -1,  0,  0,
        -0.5,  0.5,  0.5, -1,  0,  0,

         0.5,  0.5,  0.5,  1,  0,  0,
         0.5,  0.5, -0.5,  1,  0,  0,
         0.5, -0.5, -0.5,  1,  0,  0,
         0.5, -0.5, -0.5,  1,  0,  0,
         0.5, -0.5,  0.5,  1,  0,  0,
         0.5,  0.5,  0.5,  1,  0,  0,

        -0.5, -0.5, -0.5,  0, -1,  0,
         0.5, -0.5, -0.5,  0, -1,  0,
         0.5, -0.5,  0.5,  0, -1,  0,
         0.5, -0.5,  0.5,  0, -1,  0,
        -0.5, -0.5,  0.5,  0, -1,  0,
        -0.5, -0.5, -0.5,  0, -1,  0,

        -0.5,  0.5, -0.5,  0,  1,  0,
         0.5,  0.5, -0.5,  0,  1,  0,
         0.5,  0.5,  0.5,  0,  1,  0,
         0.5,  0.5,  0.5,  0,  1,  0,
        -0.5,  0.5,  0.5,  0,  1,  0,
        -0.5,  0.5, -0.5,  0,  1,  0
    };
    draw_objects(win, vertices, option);
}

// helper function to draw lighting objects
void draw_objects(GLFWwindow *win, const std::vector<GLfloat> &vertices,
        const int option, const bool rot_lamp) {

    const Shader obj_shader {shad_path + "light_shader_diffuse_01.vs",
            shad_path + "light_shader_mater_01.frag"};
    const Shader lamp_shader {shad_path + "lamp_shader_01.vs",
        shad_path + "lamp_shader_01.frag"};

    GLuint VAO_obj {}, VAO_lamp {}, VBO {};

    std::vector<GLuint> VAO_vec {VAO_obj, VAO_lamp};
    std::vector<GLuint> VBO_vec {VBO};

    gen_objects(&VAO_vec, &VBO_vec);
    make_objects(VAO_vec[0], VBO_vec[0], vertices, true, 1);
    make_objects(VAO_vec[1], VBO_vec[0], vertices, false, 0);

    light_obj_loop(win, VAO_vec, std::vector<Shader> {obj_shader, lamp_shader},
            rot_lamp, option);
}

// main loop for drawing light objects
void light_obj_loop(GLFWwindow *win, const std::vector<GLuint> &VAO,
        const std::vector<Shader> &shad, const bool rot_lamp, const int opt) {
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    const auto win_asp = float(win_w) / win_h;

    glm::vec3 lamp_pos {1, 0, 2.5};
    const std::function<void(const Shader&, const GLuint&, const glm::mat4&,
            const glm::mat4&, const glm::vec3&, const int, const int)>
        draw_fun = common_draw_light_obj;

    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        if (rot_lamp) {
            lamp_pos.x = 1 + sin(curr_time) * 2;
            lamp_pos.y = sin(curr_time * 0.5);
        }

        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;
        glfwPollEvents();
        do_movement();

        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto view = main_cam.view_matrix();
        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);

        for (int i = 0; i < 2; ++i)
            draw_fun(shad[i], VAO[i], view, proj, lamp_pos, i, opt);

        glfwSwapBuffers(win);
    }
}

// helper function for drawing colored object
void draw_light_obj(const Shader &shad, const GLuint VAO, const glm::mat4 &view,
        const glm::mat4 &proj, const glm::vec3 &lamp_pos, const int option) {
    shad.use();
    const auto idx = shad.id();

    // positions
    glUniform3f(glGetUniformLocation(idx, "light.pos"),
            lamp_pos.x, lamp_pos.y, lamp_pos.z);
    glUniform3f(glGetUniformLocation(idx, "view_pos"),
            main_cam.pos().x, main_cam.pos().y, main_cam.pos().z);

    // color
    auto light_color = glm::vec3{1, 1, 1};
    if (option == 1) {
        light_color.x = sin(glfwGetTime() * 2);
        light_color.y = sin(glfwGetTime() * 0.7);
        light_color.z = sin(glfwGetTime() * 1.3);
    }
    auto diffuse_color = light_color * glm::vec3{0.5};
    auto amb_color = diffuse_color * glm::vec3{0.37};
    if (option == 2) {
        diffuse_color = glm::vec3{1};
        amb_color = glm::vec3{1};
    }

    glUniform3f(glGetUniformLocation(idx, "light.ambient"),
            amb_color.x, amb_color.y, amb_color.z);
    glUniform3f(glGetUniformLocation(idx, "light.diffuse"),
            diffuse_color.x, diffuse_color.y, diffuse_color.z);
    glUniform3f(glGetUniformLocation(idx, "light.specular"), 1, 1, 1);

    // material
    if (option == 2) { // cyan plastic
        glUniform3f(glGetUniformLocation(idx, "mater.ambient"), 0, 0.1, 0.6);
        glUniform3f(glGetUniformLocation(idx, "mater.diffuse"), 0, 0.50980392,
                0.50980392);
        glUniform3f(glGetUniformLocation(idx, "mater.specular"), 0.50196078,
                0.50196078, 0.50196078);
    } else {
        glUniform3f(glGetUniformLocation(idx, "mater.ambient"), 1, 0.5, 0.31);
        glUniform3f(glGetUniformLocation(idx, "mater.diffuse"), 1, 0.5, 0.31);
        glUniform3f(glGetUniformLocation(idx, "mater.specular"), 0.5, 0.5, 0.5);
    }
    glUniform1f(glGetUniformLocation(idx, "mater.shininess"), 32);

    // matrices
    glm::mat4 model;
    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
            glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// helper function for drawing "lamp" object
void draw_lamp(const Shader &shad, const GLuint VAO, const glm::mat4 &view,
        const glm::mat4 &proj, const glm::vec3 &lamp_pos) {
    shad.use();
    const auto model_loc = glGetUniformLocation(shad.id(), "model");
    const auto view_loc  = glGetUniformLocation(shad.id(), "view");
    const auto proj_loc  = glGetUniformLocation(shad.id(), "proj");

    glm::mat4 model = glm::translate(glm::mat4{}, lamp_pos);
    model = glm::scale(model, glm::vec3{0.2});

    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// common helper function for drawing a light object (used for std::function)
void common_draw_light_obj(const Shader &shad, const GLuint VAO,
        const glm::mat4 &view, const glm::mat4 &proj,
        const glm::vec3 &lamp_pos, const int option, const int opt) {
    if (option == 0)
        draw_light_obj(shad, VAO, view, proj, lamp_pos, opt);
    else
        draw_lamp(shad, VAO, view, proj, lamp_pos);
}

