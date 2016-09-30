/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Advanced-OpenGL/Instancing
 *
 * Demonstration of instancing techniques:
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
static Camera main_cam {glm::vec3{0, 5, 60}};

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

// functions for generating VAO and VBO objects
void gen_objects_base(GLuint&, GLuint&, const std::vector<GLfloat>&,
        const GLuint, const GLuint);
void gen_objects(GLuint&, GLuint&, const std::vector<GLfloat>&,
        const GLuint, const GLuint);

// generate buffer for instance data
GLuint gen_instance_buf(const std::vector<glm::vec2>&);
// setting instance data
void set_instance_data(const GLuint);

// array of offsets for quads
std::vector<glm::vec2> offsets_array(const GLuint);
// array of transformation matrices
std::vector<glm::mat4> transform_mats(const GLuint, const double, const double);
// set transformation matrices as an instance vertex attribute
void bind_mat4(Model& m, const std::vector<glm::mat4>& v);
// get shader depending of the option value
Shader choose_shader(const int = 0);
// draw objects with points
void draw_objects(const Shader&, const GLuint);
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
        "This program demonstrates the use of instancing:\n" <<
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
    static constexpr char num_options {'4'};
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
        "0:\t100 colourful square (default)\n" <<
        "1:\t100 colourful with varying size and using instancing\n" <<
        "2:\tplanet with asteroids (without instancing)\n" <<
        "3:\tplanet with asteroids (with instancing)\n";
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Instancing", nullptr, nullptr);
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
void gen_objects_base(GLuint &vao, GLuint &vbo, const std::vector<GLfloat> &v,
        const GLuint stride, const GLuint offset) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(v), v.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat),
            0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            stride * sizeof(GLfloat), (GLvoid*)(offset * sizeof(GLfloat)));
}

// generate objects: appending glBindVertexArray(0);
void gen_objects(GLuint &vao, GLuint &vbo, const std::vector<GLfloat> &v,
        const GLuint stride, const GLuint offset) {
    gen_objects_base(vao, vbo, v, stride, offset);
    glBindVertexArray(0);
}

// generate buffer for instance data
GLuint gen_instance_buf(const std::vector<glm::vec2>& v) {
    GLuint inst_vbo;
    glGenBuffers(1, &inst_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, inst_vbo);
    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(v), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inst_vbo;
}

// setting instance data
void set_instance_data(const GLuint vbo) {
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
            (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // inform OpenGL about instanced vertex attribute
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

// quads
std::vector<GLfloat> quad_vertices() {
    return std::vector<GLfloat> {
        // positions    colors
        -0.05,  0.05,  1, 0, 0,
         0.05, -0.05,  0, 1, 0,
        -0.05, -0.05,  0, 0, 1,

        -0.05,  0.05,  1, 0, 1,
         0.05, -0.05,  1, 1, 0,
         0.05,  0.05,  0, 1, 1
    };
}

// get shader depending of the option value
Shader choose_shader(const int option) {
    switch (option) {
        case 3: case 2:
            return Shader {shad_path + "model_loading_03.vs",
                shad_path + "model_loading_01.frag"};
        case 1:
            return Shader {shad_path + "instancing_02.vs",
                shad_path + "geom_01.frag"};
        case 0:
        default:
            return Shader {shad_path + "instancing_01.vs",
                shad_path + "geom_01.frag"};
    }
}

// generate offsets for the quads
std::vector<glm::vec2> offsets_array(const GLuint n) {
    const double side {std::sqrt(n)};
    std::vector<glm::vec2> offset_vec(n);
    GLfloat offset_value = 0.1;
    int idx = -1;
    for (GLint i = -side; i < side; i += 2)
        for (GLint j = -side; j < side; j += 2)
            offset_vec[++idx] = glm::vec2 {i / side + offset_value,
                j / side + offset_value};
    return offset_vec;
}

// array of random transformation matrices
std::vector<glm::mat4> transform_mats(const GLuint n, const double radius,
        const double offset) {
    std::vector<glm::mat4> model_mats(n);
    std::srand(glfwGetTime()); // random seed
    for (GLuint i {0}; i < n; ++i) {
        // part 1: translation
        const double angle {double(i) / n * 360.0};
        double shift {(rand() % int(offset * 200)) / 100.0 - offset};
        const double x {sin(angle) * radius + shift};
        shift = {(rand() % int(offset * 200)) / 100.0 - offset};
        // less impact on y axes to have a more flat ring
        const double y {shift * 0.4 - offset / 10.0};
        shift = {(rand() % int(offset * 200)) / 100.0 - offset};
        const double z {cos(angle) * radius + shift};
        glm::mat4 m {glm::translate(glm::mat4{}, glm::vec3{x, y, z})};

        // part 2: scale
        const GLfloat scale_val = (rand() % 20) / 100.0 + 0.05;
        m = glm::scale(m, glm::vec3{scale_val});

        // part 2: rotation
        const GLfloat rot_angle = rand() % 360;
        m = glm::rotate(m, rot_angle, glm::vec3(0.4, 0.6, 0.8));

        // add to the array of matrices
        model_mats[i] = m;
    }
    return model_mats;
}

// set transformation matrices as an instance vertex attribute
void bind_mat4(Model& m, const std::vector<glm::mat4>& v) {
    for (GLuint i {0}; i < m.num_meshes(); ++i) {
        glBindVertexArray(m.mesh_vao(i));

        GLuint buf;
        glGenBuffers(1, &buf);

        glBindBuffer(GL_ARRAY_BUFFER, buf);
        glBufferData(GL_ARRAY_BUFFER, size_in_bytes(v), v.data(),
                GL_STATIC_DRAW);

        const auto mat4_size = sizeof(glm::mat4);
        const auto vec4_size = sizeof(glm::vec4);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, mat4_size,
                (GLvoid*)(0));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, mat4_size,
                (GLvoid*)(vec4_size));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, mat4_size,
                (GLvoid*)(vec4_size << 1));

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, mat4_size,
                (GLvoid*)(3 * vec4_size));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
}

// main drawing loop
void game_loop(GLFWwindow* win, const int option) {
    if (option < 2)
        game_loop_objects(win, option, choose_shader(option));
    else
        game_loop_model(win, option, choose_shader(option));
}

// loop for drawing objects
void game_loop_objects(GLFWwindow* win, const int option, const Shader& shad) {
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw_objects(shad, option);

        glfwSwapBuffers(win);
    }
}

void handle_camera_matrices(const Shader& s1, const Shader& s2,
        const float win_asp, const int option) {
    const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
            10000.0f);
    const auto idx1 = s1.id(), idx2 = s2.id();
    s1.use();
    glUniformMatrix4fv(glGetUniformLocation(idx1, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));
    if (option == 3) {
        s2.use();
        glUniformMatrix4fv(glGetUniformLocation(idx2, "proj"), 1,
                GL_FALSE, glm::value_ptr(proj));
        s1.use();
    }

    const auto view = main_cam.view_matrix();
    glUniformMatrix4fv(glGetUniformLocation(idx1, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    if (option == 3) {
        s2.use();
        glUniformMatrix4fv(glGetUniformLocation(idx2, "view"), 1,
                GL_FALSE, glm::value_ptr(view));
        s1.use();
    }

    const auto mat_mod = glm::scale(glm::translate(glm::mat4{},
                glm::vec3{0, -3, 0}), glm::vec3{4});
    glUniformMatrix4fv(glGetUniformLocation(idx1, "model"), 1, GL_FALSE,
            glm::value_ptr(mat_mod));
}

// loop for drawing the planet and asteroids
void game_loop_model(GLFWwindow* win, const int option, const Shader& shad) {
    Model m_planet {model_path + "planet/planet.obj"};
    Model m_rock {model_path + "rock/rock.obj"};
    GLuint n {};
    double radius {}, offset {};
    if (option == 2) {
        n = 2000;
        radius = 50;
        offset = 2.5;
    } else {
        main_cam = Camera {glm::vec3{0, 5, 200}};
        n = 10000;
        radius = 150;
        offset = 25;
    }
    static const auto mod_mats = transform_mats(n, radius, offset);

    Shader rock_shad {shad_path + "model_loading_04.vs",
        shad_path + "model_loading_01.frag"};
    if (option == 3)
        bind_mat4(m_rock, mod_mats);

    const static auto win_asp = window_aspect_ratio(win);
    const auto idx = shad.id();
    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;
        glfwPollEvents();
        do_movement();
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        handle_camera_matrices(shad, rock_shad, win_asp, option);

        m_planet.draw(shad);
        if (option == 2) { // drawing rocks
            for (GLuint i {0}; i < n; ++i) {
                glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1,
                        GL_FALSE, glm::value_ptr(mod_mats[i]));
                m_rock.draw(shad);
            }
        } else {
            rock_shad.use();
            glBindTexture(GL_TEXTURE_2D, m_rock.texture_id(0));
            for (GLuint i {0}; i < m_rock.num_meshes(); ++i) {
                glBindVertexArray(m_rock.mesh_vao(i));
                glDrawElementsInstanced(GL_TRIANGLES,
                        m_rock.num_mesh_vertices(i), GL_UNSIGNED_INT, 0, n);
                glBindVertexArray(0);
            }
        }
        glfwSwapBuffers(win);
    }
}

// draw objects with geometry shader defined by points
void draw_objects(const Shader& shad, const GLuint option) {
    // number of objects (quads)
    constexpr GLuint n {100};
    shad.use();

    GLuint vao{}, vbo{};
    if (option == 0) {
        gen_objects(vao, vbo, quad_vertices(), 5, 2);
        const auto offset_arr = offsets_array(n);
        const auto idx = shad.id();
        for (GLuint i {0}; i < n; ++i) {
            const GLint loc = glGetUniformLocation(idx,
                    std::string{"offsets_arr[" + std::to_string(i) + "]"}.
                    c_str());
            glUniform2f(loc, offset_arr[i].x, offset_arr[i].y);
        }
    } else {
        GLuint inst_vbo {gen_instance_buf(offsets_array(n))};
        gen_objects_base(vao, vbo, quad_vertices(), 5, 2);
        set_instance_data(inst_vbo);
    }

    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, n);
    glBindVertexArray(0);
}

