/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Advanced-OpenGL/Face-culling
 *
 * Blending options demonstration
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
// generate VAO, VBO, EBO and bind the objects with vertex data
void load_object(GLuint&, GLuint&, const std::vector<GLfloat>&, const GLuint);

// loading textures
GLuint load_texture(const std::string&, const GLboolean = false);

// define vertices
std::vector<GLfloat> cube_vertices_cull_ccw();
std::vector<GLfloat> cube_vertices_cull_cw();

// drawing objects
void face_cull_test(GLFWwindow*, const int = 0);
void draw_object(const Shader&, const GLuint, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::mat4&, const GLuint);
void game_loop(GLFWwindow*, const GLuint, const GLuint, const size_t,
        const Shader&, const int);

// function to compute aspect ratio of screen's width and height
float window_aspect_ratio(GLFWwindow*);
// function to compute sizeof elements lying in the vector container
template <class T>
constexpr size_t size_in_bytes(const std::vector<T> &v) {
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
        "This program demonstrates a couple of face culling options:\n" <<
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
    static constexpr char num_options {'3'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char < num_options) {
        switch (inp_char - '0') {
            case 2:
                face_cull_test(win, 2);
                break;
            case 1:
                face_cull_test(win, 1);
                break;
            case 0:
            default:
                face_cull_test(win, 0);
        }
    } else {
        std::cerr << "Wrong input: drawing default scene\n";
        face_cull_test(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tcubes with back face culling (default)\n" <<
        "1:\tcubes with front face culling (only back faces visible)\n";
    face_cull_test(win, 0);
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


    // debugging
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        std::cout << "debugging enabled\n";
    else
        std::cout << "no debugging\n";


    // create a window object
    GLFWwindow *win = glfwCreateWindow(w, h, "Face culling", nullptr, nullptr);
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

    // enable depth test and face culling
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);

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
 * Generate objects like Vertex Array Objects, Vertex Buffer Object and bind
 * them with vertex data
 */
void load_object(GLuint &VAO, GLuint &VBO, const std::vector<GLfloat> &vertices,
        const GLuint stride) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(vertices), vertices.data(),
            GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat),
            (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat),
            (GLvoid*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
}

/*
 * Binding textures with the image data
 * If image uses the alpha value then GL_RGBA parameter is used instead of
 * GL_RGB, as well as the wrap parameter is changed to GL_CLAMP_TO_EDGE
 */
GLuint load_texture(const std::string& img_fn, const GLboolean alpha) {
    GLuint tex_id;
    glGenTextures(1, &tex_id);

    //const auto load_type = alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB;
    const auto img_type = alpha ? GL_RGBA : GL_RGB;
    const auto wrap_type = alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT;

    int img_w, img_h;
    unsigned char *img = SOIL_load_image(img_fn.c_str(), &img_w, &img_h, 0,
            alpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);

    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, img_type, img_w, img_h, 0, img_type,
            GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(img);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex_id;
}

// cube object vertices with counter-clockwise order for face culling
std::vector<GLfloat> cube_vertices_cull_ccw() {
    return std::vector<GLfloat> {
        // Back face
        -0.5, -0.5, -0.5,  0, 0, // Bottom-left
         0.5,  0.5, -0.5,  1, 1, // top-right
         0.5, -0.5, -0.5,  1, 0, // bottom-right
         0.5,  0.5, -0.5,  1, 1, // top-right
        -0.5, -0.5, -0.5,  0, 0, // bottom-left
        -0.5,  0.5, -0.5,  0, 1, // top-left
        // Front face
        -0.5, -0.5,  0.5,  0, 0, // bottom-left
         0.5, -0.5,  0.5,  1, 0, // bottom-right
         0.5,  0.5,  0.5,  1, 1, // top-right
         0.5,  0.5,  0.5,  1, 1, // top-right
        -0.5,  0.5,  0.5,  0, 1, // top-left
        -0.5, -0.5,  0.5,  0, 0, // bottom-left
        // Left face
        -0.5,  0.5,  0.5,  1, 0, // top-right
        -0.5,  0.5, -0.5,  1, 1, // top-left
        -0.5, -0.5, -0.5,  0, 1, // bottom-left
        -0.5, -0.5, -0.5,  0, 1, // bottom-left
        -0.5, -0.5,  0.5,  0, 0, // bottom-right
        -0.5,  0.5,  0.5,  1, 0, // top-right
        // Right face
         0.5,  0.5,  0.5,  1, 0, // top-left
         0.5, -0.5, -0.5,  0, 1, // bottom-right
         0.5,  0.5, -0.5,  1, 1, // top-right
         0.5, -0.5, -0.5,  0, 1, // bottom-right
         0.5,  0.5,  0.5,  1, 0, // top-left
         0.5, -0.5,  0.5,  0, 0, // bottom-left
        // Bottom face
        -0.5, -0.5, -0.5,  0, 1, // top-right
         0.5, -0.5, -0.5,  1, 1, // top-left
         0.5, -0.5,  0.5,  1, 0, // bottom-left
         0.5, -0.5,  0.5,  1, 0, // bottom-left
        -0.5, -0.5,  0.5,  0, 0, // bottom-right
        -0.5, -0.5, -0.5,  0, 1, // top-right
        // Top face
        -0.5,  0.5, -0.5,  0, 1, // top-left
         0.5,  0.5,  0.5,  1, 0, // bottom-right
         0.5,  0.5, -0.5,  1, 1, // top-right
         0.5,  0.5,  0.5,  1, 0, // bottom-right
        -0.5,  0.5, -0.5,  0, 1, // top-left
        -0.5,  0.5,  0.5,  0, 0  // bottom-left
    };
}

// cube object vertices with clockwise order for face culling
std::vector<GLfloat> cube_vertices_cull_cw() {
    return std::vector<GLfloat> {
        // Back face
        -0.5, -0.5, -0.5,  0, 0, // Bottom-left
         0.5, -0.5, -0.5,  1, 0, // bottom-right
         0.5,  0.5, -0.5,  1, 1, // top-right
         0.5,  0.5, -0.5,  1, 1, // top-right
        -0.5,  0.5, -0.5,  0, 1, // top-left
        -0.5, -0.5, -0.5,  0, 0, // bottom-left
        // Front face
        -0.5, -0.5,  0.5,  0, 0, // bottom-left
         0.5,  0.5,  0.5,  1, 1, // top-right
         0.5, -0.5,  0.5,  1, 0, // bottom-right
         0.5,  0.5,  0.5,  1, 1, // top-right
        -0.5, -0.5,  0.5,  0, 0, // bottom-left
        -0.5,  0.5,  0.5,  0, 1, // top-left
        // Left face
        -0.5,  0.5,  0.5,  1, 0, // top-right
        -0.5, -0.5, -0.5,  0, 1, // bottom-left
        -0.5,  0.5, -0.5,  1, 1, // top-left
        -0.5, -0.5, -0.5,  0, 1, // bottom-left
        -0.5,  0.5,  0.5,  1, 0, // top-right
        -0.5, -0.5,  0.5,  0, 0, // bottom-right
        // Right face
         0.5,  0.5,  0.5,  1, 0, // top-left
         0.5,  0.5, -0.5,  1, 1, // top-right
         0.5, -0.5, -0.5,  0, 1, // bottom-right
         0.5, -0.5, -0.5,  0, 1, // bottom-right
         0.5, -0.5,  0.5,  0, 0, // bottom-left
         0.5,  0.5,  0.5,  1, 0, // top-left
        // Bottom face
        -0.5, -0.5, -0.5,  0, 1, // top-right
         0.5, -0.5,  0.5,  1, 0, // bottom-left
         0.5, -0.5, -0.5,  1, 1, // top-left
         0.5, -0.5,  0.5,  1, 0, // bottom-left
        -0.5, -0.5, -0.5,  0, 1, // top-right
        -0.5, -0.5,  0.5,  0, 0, // bottom-right
        // Top face
        -0.5,  0.5, -0.5,  0, 1, // top-left
         0.5,  0.5, -0.5,  1, 1, // top-right
         0.5,  0.5,  0.5,  1, 0, // bottom-right
        -0.5,  0.5, -0.5,  0, 1, // top-left
         0.5,  0.5,  0.5,  1, 0, // bottom-right
        -0.5,  0.5,  0.5,  0, 0  // bottom-left
    };
}

// get coordinates of cubes objects
std::vector<glm::vec3> cubes_positions() {
    return {
        glm::vec3{-1, 0, -1},
        glm::vec3{ 2, 0,  0}
    };
}

// draw boxes on a floor
void face_cull_test(GLFWwindow *win, const int option) {
    static const auto verts = cube_vertices_cull_ccw();

    const GLuint stride {5};
    GLuint VAO_cube {}, VBO_cube {};
    load_object(VAO_cube, VBO_cube, verts, stride);

    // loading and mapping the texture
    const GLuint tex_cube = load_texture(tex_path + "pattern4diffuseblack.jpg");

    static const Shader obj_shader {shad_path + "depth_test_01.vs",
        shad_path + "depth_test_01.frag"};

    game_loop(win, VAO_cube, tex_cube, verts.size() / stride,
            Shader {shad_path + "depth_test_01.vs",
            shad_path + "depth_test_01.frag"}, option);
}

// main loop for drawing light objects
void game_loop(GLFWwindow *win, const GLuint VAO, const GLuint tex_map,
        const size_t num_verts, const Shader &shad, const int option) {
    const auto win_asp = window_aspect_ratio(win);

    /*
     * Make only the back faces visible (front faces culled) by either setting
     * the option for glFrontFace to consider clockwise order instead of default
     * counter-clockwise or using glCullFace to tell OpenGL which faces are to
     * be discarded. There is also the third option: use cube_vertices_cull_cw()
     * function to load the vertices in already clockwise order (then do not
     * change the default face cull parameters)
     */
    if (option == 1) {
        //glFrontFace(GL_CW);
        glCullFace(GL_FRONT);
    }

    static const auto cubes_pos = cubes_positions();
    const auto num_pos = cubes_pos.size();
    std::vector<glm::mat4> mod_mats (num_pos);
    for (size_t i {0}; i < num_pos; ++i)
        mod_mats[i] = glm::translate(glm::mat4{}, cubes_pos[i]);

    glClearColor(0.05, 0.05, 0.05, 1);
    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;

        glfwPollEvents();
        do_movement();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto view = main_cam.view_matrix();
        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);

        for (size_t i {0}; i < num_pos; ++i)
            draw_object(shad, VAO, tex_map, view, proj, mod_mats[i], num_verts);

        glfwSwapBuffers(win);
    }
}

// drawing an object
void draw_object(const Shader &shad, const GLuint VAO, const GLuint tex_map,
        const glm::mat4 &view, const glm::mat4 &proj, const glm::mat4 &mod,
        const GLuint num_verts) {
    shad.use();
    const auto idx = shad.id();

    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
            glm::value_ptr(mod));

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, tex_map);

    glDrawArrays(GL_TRIANGLES, 0, num_verts);

    glBindVertexArray(0);
}

