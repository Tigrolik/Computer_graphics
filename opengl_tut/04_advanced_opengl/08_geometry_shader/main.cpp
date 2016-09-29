/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Advanced-OpenGL/Geometry-Shader
 *
 * Demonstration of geometry shaders:
 *     - drawing objects
 *     - "exploding" model and drawing its normal vectors
 *
 * The Shader class has been modified:
 *     - added two constructors to accept three files
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

// generate VAO and VBOs
void gen_objects(std::vector<GLuint>*, std::vector<GLuint>*);
void make_objects(const GLuint, const GLuint, const std::vector<GLfloat>&,
        const GLuint, const GLuint, const bool = false);

// four points
std::vector<GLfloat> points_01();
// four points with color components
std::vector<GLfloat> points_02();

// get shader depending of the option value
Shader choose_shader(const int = 0);
// draw objects with points
void draw_objects(const Shader&, const GLuint, const GLuint = 4);
// draw four green houses
void draw_green_houses(const Shader&, const GLuint);
// drawing a model
void draw_model(Model&, const Shader&, const glm::mat4&, const glm::mat4&,
        const glm::mat4&, const int);
// drawing loop
void game_loop(GLFWwindow*, const int = 0);
// drawing objects
void game_loop_objects(GLFWwindow*, const int, const Shader&);
// drawing model
void game_loop_model(GLFWwindow*, const int, const Shader&);

// function to compute aspect ratio of screen's width and height
float window_aspect_ratio(GLFWwindow*);
// function to compute sizeof elements lying in the vector container
template <class T>
constexpr size_t size_in_bytes(const std::vector<T>& v) {
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
        "This program demonstrates the use of geometry shaders:\n" <<
        "keys A/D, left/right arrow keys control side camera movement\n" <<
        "up/down arrow keys - up and down, W/S - depth\n" <<
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
    static constexpr char num_options {'5'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char < num_options) {
        game_loop(win, inp_char - '0');
    } else {
        std::cerr << "Wrong input: drawing default scene\n";
        game_loop(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tfour big green dots (default)\n" <<
        "1:\tfour green houses (two in wireframe mode)\n" <<
        "2:\tfour color houses\n" <<
        "3:\t\"exploding\" crisis nanosuit\n" <<
        "4:\tcrisis nanosuit with normal vectors\n";
    game_loop(win, 0);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Geometry Shader",
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

    // allow modifying the size of pixel
    glEnable(GL_PROGRAM_POINT_SIZE);

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
        const std::vector<GLfloat> &vertices, const GLuint stride,
        const GLuint offset, const bool second_attr) {
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(vertices), vertices.data(),
            GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat),
            0);

    if (second_attr) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                stride * sizeof(GLfloat), (GLvoid*)(offset * sizeof(GLfloat)));
    }

    glBindVertexArray(0);
}

// four points
std::vector<GLfloat> points_01() {
    return std::vector<GLfloat> {
        -0.5,  0.5, // top-left
         0.5,  0.5, // top-right
         0.5, -0.5, // bottom-right
        -0.5, -0.5  // bottom-left
    };
}

// four points with color components
std::vector<GLfloat> points_02() {
    return std::vector<GLfloat> {
        -0.5,  0.5, 1, 0, 0, // top-left
         0.5,  0.5, 0, 1, 0, // top-right
         0.5, -0.5, 0, 0, 1, // bottom-right
        -0.5, -0.5, 1, 1, 0 // bottom-left
    };
}

// get shader depending of the option value
Shader choose_shader(const int option) {
    switch (option) {
        case 4:
            return Shader {shad_path + "geom_04.vs", shad_path + "geom_05.geom",
                    shad_path + "ubo_yellow_01.frag"};
        case 3:
            return Shader {shad_path + "geom_03.vs", shad_path + "geom_04.geom",
                shad_path + "model_loading_01.frag"};
        case 2:
            return Shader {shad_path + "geom_02.vs",
                shad_path + "geom_03.geom", shad_path + "geom_01.frag"};
        case 1:
            return Shader {shad_path + "geom_01.vs",
                shad_path + "geom_02.geom", shad_path + "ubo_green_01.frag"};
        case 0:
        default:
            return Shader {shad_path + "geom_01.vs",
                shad_path + "geom_01.geom", shad_path + "ubo_green_01.frag"};
    }
}

// draw objects with geometry shader defined by points
void draw_objects(const Shader& shad, const GLuint VAO, const GLuint num_p) {
    shad.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, num_p);
    glBindVertexArray(0);
}

// draw four green houses: two are in wireframe mode
void draw_green_houses(const Shader& shad, const GLuint VAO) {
    shad.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
    glDrawArrays(GL_POINTS, 3, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_POINTS, 1, 2);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);
}

// main drawing loop
void game_loop(GLFWwindow* win, const int option) {
    if (option < 3)
        game_loop_objects(win, option, choose_shader(option));
    else
        game_loop_model(win, option, choose_shader(option));
}

// loop for drawing objects
void game_loop_objects(GLFWwindow* win, const int option, const Shader& shad) {
    constexpr auto num_obj = 1;
    std::vector<GLuint> VAO_vec (num_obj);
    std::vector<GLuint> VBO_vec (num_obj);
    gen_objects(&VAO_vec, &VBO_vec);

    if (option < 2)
        make_objects(VAO_vec[0], VBO_vec[0], points_01(), 2, 2);
    else
        make_objects(VAO_vec[0], VBO_vec[0], points_02(), 5, 2, true);

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (option == 1)
            draw_green_houses(shad, VAO_vec[0]);
        else
            draw_objects(shad, VAO_vec[0], 4);

        glfwSwapBuffers(win);
    }
}

// loop for drawing the model
void game_loop_model(GLFWwindow* win, const int option, const Shader& shad) {

    if (option == 9)
        std::cout << "wtf\n";

    Model m {model_path + "crysis_nanosuit_refl/nanosuit.obj"};

    const auto win_asp = window_aspect_ratio(win);

    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;

        glfwPollEvents();
        do_movement();

        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);
        const auto view = main_cam.view_matrix();
        const auto mat_mod = glm::scale(glm::translate(glm::mat4{},
                    glm::vec3{0, -8.2, -12}), glm::vec3{1.1});

        if (option == 4)
            draw_model(m, Shader {shad_path + "model_loading_01.vs", shad_path
                    + "model_loading_01.frag"}, proj, view, mat_mod, option);
        draw_model(m, shad, proj, view, mat_mod, option);

        glfwSwapBuffers(win);
    }
}

// drawing the model
void draw_model(Model& m, const Shader& shad, const glm::mat4& proj,
        const glm::mat4& view, const glm::mat4& mat_mod, const int option) {
    shad.use();
    const auto idx = shad.id();

    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
            glm::value_ptr(mat_mod));

    if (option == 3)
        glUniform1f(glGetUniformLocation(idx, "time_value"), glfwGetTime());

    m.draw(shad);
}

