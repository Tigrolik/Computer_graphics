/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Model-Loading/Model
 *
 * Loading a model using Assimp, the compilation now needs the option -lassimp
 * which has been added to the Makefile
 *
 */

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

// GL math
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Model.h"

// paths to the folder where we keep shaders, textures and models: global vars
static const std::string shad_path {"../shaders/"};
static const std::string tex_path {"../images/"};
static const std::string model_path {"../models/"};

// tracking which keys have been pressed/released (for smooth movement)
static bool keys[1024];
// global values to keep track of time between the frames
static GLfloat delta_frame_time = 0;
static GLfloat last_frame_time  = 0;
// last cursor position
static GLfloat last_x = 0, last_y = 0;
// avoid sudden jump of the camera at the beginning
static bool first_mouse_move {true};
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
// main loop
void game_loop(GLFWwindow*, Model&, const Shader&, const int);

// drawing a model
void draw_model(GLFWwindow*, Model&, const int);

// process user input
void process_input(GLFWwindow*, Model&, const std::string&);
// display menu of possible actions and process them
void show_menu(GLFWwindow*, Model&, const std::string&);

void set_dir_light(const GLuint, const int);
void set_point_lights(const GLuint, const std::vector<glm::vec3>&, const int);
void set_spot_light(const GLuint, const int);

// here goes the main()
int main(int argc, char *argv[]) try {

    static constexpr GLuint width {800}, height {600};
    last_x = width >> 1;
    last_y = height >> 1;
    GLFWwindow *win = init(width, height);

    std::cout <<
        "----------------------------------------------------------------\n" <<
        "This program demonstrates model loading:\n" <<
        "keys A/D, left/right arrow keys control side camera movement\n" <<
        "keys W/S - up and down, arrows up/down - depth\n" <<
        "mouse can also be used to change view/zoom (scroll)\n" <<
        "----------------------------------------------------------------\n";

    // draw model
    Model nanosuit_model {model_path + "crysis_nanosuit/nanosuit.obj"};

    if (argc > 1)
        process_input(win, nanosuit_model, argv[1]);
    else
        show_menu(win, nanosuit_model, argv[0]);

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
void process_input(GLFWwindow *win, Model& model, const std::string &inp) {
    static constexpr char num_options {'2'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char < num_options) {
        switch (inp_char - '0') {
            case 1:
                draw_model(win, model, 1);
                break;
            case 0:
            default:
                draw_model(win, model, 0);
        }
    } else {
        std::cerr << "Wrong input: drawing default model\n";
        draw_model(win, model, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, Model& model, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tmodel (default)\n" <<
        "1:\tmodel with lighting\n";
    draw_model(win, model, 0);
}

/*
 * Function to initialize stuff. With comparison to the first tutorial here we
 * put more stuff inside a single function, though trying not to make it too
 * long (keeping the idea of fitting a single function on a screen in mind)
 */
GLFWwindow* init(const GLuint w, const GLuint h) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *win = glfwCreateWindow(w, h, "Model loading", nullptr, nullptr);
    if (win == nullptr)
        throw std::runtime_error {"Failed to create GLFW window"};
    glfwMakeContextCurrent(win);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error {"Failed to initialize GLEW"};

    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, mouse_callback);
    glfwSetScrollCallback(win, scroll_callback);

    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glViewport(0, 0, w, h);
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

// drawing the model
void draw_model(GLFWwindow *win, Model& m, const int option) {
    switch (option) {
        case 1:
            game_loop(win, m, Shader {shad_path + "light_shader_direct_01.vs",
                    shad_path + "model_loading_02.frag"}, option);
            break;
        default:
            game_loop(win, m, Shader {shad_path + "model_loading_01.vs",
                    shad_path + "model_loading_01.frag"}, option);
    }
}

// main loop for drawing objects
void game_loop(GLFWwindow *win, Model& model, const Shader& shad,
        const int option) {
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    const auto win_asp = float(win_w) / win_h;

    static const std::vector<glm::vec3> lamps_pos = {
        glm::vec3{2.7, 2.2, 2}, glm::vec3{2.3, 3.3, 4},
        glm::vec3{4, 2, 12}, glm::vec3{0, 0, 1}
    };

    const glm::vec3 lamp_pos {1, 0, -1.5};
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!glfwWindowShouldClose(win)) {

        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;

        glfwPollEvents();
        do_movement();

        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shad.use();
        const auto idx = shad.id();

        // set lighting
        if (option > 0) {
            set_dir_light(idx, option);
            set_point_lights(idx, lamps_pos, option);
            set_spot_light(idx, option);
        }

        // matrices
        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);
        glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
                glm::value_ptr(proj));
        const auto view = main_cam.view_matrix();
        glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
                glm::value_ptr(view));

        glm::mat4 mat_model = glm::scale(glm::translate(glm::mat4{},
                    glm::vec3{0, -1.75, 0}), glm::vec3{0.2});
        glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
                glm::value_ptr(mat_model));

        // drawing the model
        model.draw(shad);

        glfwSwapBuffers(win);
    }
}

// set directional light
void set_dir_light(const GLuint idx, const int option) {
    // directional light
    glUniform3f(glGetUniformLocation(idx, "dir_light.direction"),
            -0.2, -1, -0.3);
    switch (option) {
        case 1:
            glUniform3f(glGetUniformLocation(idx, "dir_light.ambient"),
                    0.5, 0.5, 0.5);
            glUniform3f(glGetUniformLocation(idx, "dir_light.diffuse"),
                    1, 1, 1);
            glUniform3f(glGetUniformLocation(idx, "dir_light.specular"),
                    1, 1, 1);
            break;
        default:
            glUniform3f(glGetUniformLocation(idx, "dir_light.ambient"),
                    0.1, 0.1, 0.1);
            glUniform3f(glGetUniformLocation(idx, "dir_light.diffuse"),
                    0.1, 0.1, 0.1);
            glUniform3f(glGetUniformLocation(idx, "dir_light.specular"),
                    0.8, 0.8, 0.8);
    };
}

// set the point lights
void set_point_lights(const GLuint idx,
        const std::vector<glm::vec3> &lamps_pos, const int) {
    for (GLuint i = 0; i < lamps_pos.size(); ++i) {
        const std::string ins {"point_lights[" + std::to_string(i) + "]"};
        glUniform3f(glGetUniformLocation(idx, (ins + ".pos").c_str()),
                lamps_pos[i].x, lamps_pos[i].y, lamps_pos[i].z);
        glUniform3f(glGetUniformLocation(idx, (ins + ".ambient").c_str()),
                0.5, 0.5, 0.5);
        glUniform3f(glGetUniformLocation(idx, (ins + ".diffuse").c_str()),
                0.8, 0.8, 0.8);
        glUniform3f(glGetUniformLocation(idx, (ins + ".specular").c_str()),
                1, 1, 1);
        glUniform1f(glGetUniformLocation(idx, (ins + ".constant_term").c_str()),
                1);
        glUniform1f(glGetUniformLocation(idx, (ins +
                        ".linear_term").c_str()), 0.09);
        glUniform1f(glGetUniformLocation(idx,
                    (ins + ".quadratic_term").c_str()), 0.032);
    }
}

// set the spot light (flashlight)
void set_spot_light(const GLuint idx, const int option) {
    // idx: shader program's id
    glUniform3f(glGetUniformLocation(idx, "spot_light.pos"),
            main_cam.pos().x, main_cam.pos().y, main_cam.pos().z);
    glUniform3f(glGetUniformLocation(idx, "spot_light.direction"),
            main_cam.front().x, main_cam.front().y, main_cam.front().z);
    glUniform3f(glGetUniformLocation(idx, "spot_light.ambient"), 0, 0, 0);
    switch (option) {
        case 1:
            glUniform3f(glGetUniformLocation(idx, "spot_light.diffuse"),
                    0.8, 0.8, 0.8);
            glUniform3f(glGetUniformLocation(idx, "spot_light.specular"),
                    0.8, 0.8, 0.8);
            break;
        default:
            glUniform3f(glGetUniformLocation(idx, "spot_light.diffuse"),
                    0.1, 0.1, 0.1);
            glUniform3f(glGetUniformLocation(idx, "spot_light.specular"),
                    0.1, 0.1, 0.1);
    };
    glUniform1f(glGetUniformLocation(idx, "spot_light.constant_term"), 1);
    glUniform1f(glGetUniformLocation(idx, "spot_light.linear_term"), 0.09);
    glUniform1f(glGetUniformLocation(idx, "spot_light.quadratic_term"),
            0.032);
    glUniform1f(glGetUniformLocation(idx, "spot_light.cutoff"),
            glm::cos(glm::radians(12.5)));
    glUniform1f(glGetUniformLocation(idx, "spot_light.outer_cutoff"),
            glm::cos(glm::radians(15.5)));
}
