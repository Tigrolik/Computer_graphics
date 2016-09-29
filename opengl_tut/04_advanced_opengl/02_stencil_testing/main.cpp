/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Advanced-OpenGL/Stencil-testing
 *
 * Stencil options demonstration
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
        const bool, const int, const GLuint);

// init textures
void gen_textures(std::vector<GLuint>&);
void make_textures(const GLuint, const std::string&,
        const GLenum, const GLenum);

// drawing objects
void stencil_test(GLFWwindow*, const int = 0);
void draw_objects(GLFWwindow*, const std::vector<GLfloat>&,
        const std::vector<GLfloat>&, const int = 0);
void draw_object(const Shader&, const GLuint, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::mat4&, const GLuint);
void game_loop(GLFWwindow*, const std::vector<GLuint>&,
        const std::vector<GLuint>&, const std::vector<size_t>&,
        const Shader&, const int);

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
        "This program demonstrates various stencil options:\n" <<
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
                stencil_test(win, 2);
                break;
            case 1:
                stencil_test(win, 1);
                break;
            case 0:
            default:
                stencil_test(win, 0);
        }
    } else {
        std::cerr << "Wrong input: drawing default scene\n";
        stencil_test(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\toutlined cubes: GL_REPLACE (default)\n" <<
        "1:\toutlined cubes: GL_INVERT\n" <<
        "2:\toutlined cubes: GL_INCR_WRAP\n";
    stencil_test(win, 0);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Stencil testing", nullptr,
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

    // enable depth and stencil testing for nice 3D output
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

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
        const std::vector<GLfloat> &vertices, const GLuint stride) {
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_of_elements(vertices), vertices.data(),
            GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat),
            (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat),
            (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

// exploiting std::vector to initialize several textures
void gen_textures(std::vector<GLuint> &tex) {
    for (auto &x: tex)
        glGenTextures(1, &x);
}

/*
 * Binding textures with the image data
 */
void make_textures(const GLuint tex, const std::string& img_fn,
        const std::vector<GLenum> &wrap, const std::vector<GLenum> &filter) {
    int img_w, img_h;
    unsigned char *img = SOIL_load_image(img_fn.c_str(), &img_w, &img_h, 0,
            SOIL_LOAD_RGB);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, GL_RGB,
            GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(img);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter[1]);

    glBindTexture(GL_TEXTURE_2D, 0);
}


// draw boxes with outline on a floor
void stencil_test(GLFWwindow *win, const int option) {
    static const std::vector<GLfloat> cube_verts {
        // pos          tex coords
        -0.5, -0.5, -0.5,  0, 0,
         0.5, -0.5, -0.5,  1, 0,
         0.5,  0.5, -0.5,  1, 1,
         0.5,  0.5, -0.5,  1, 1,
        -0.5,  0.5, -0.5,  0, 1,
        -0.5, -0.5, -0.5,  0, 0,
        -0.5, -0.5,  0.5,  0, 0,
         0.5, -0.5,  0.5,  1, 0,
         0.5,  0.5,  0.5,  1, 1,
         0.5,  0.5,  0.5,  1, 1,
        -0.5,  0.5,  0.5,  0, 1,
        -0.5, -0.5,  0.5,  0, 0,
        -0.5,  0.5,  0.5,  1, 0,
        -0.5,  0.5, -0.5,  1, 1,
        -0.5, -0.5, -0.5,  0, 1,
        -0.5, -0.5, -0.5,  0, 1,
        -0.5, -0.5,  0.5,  0, 0,
        -0.5,  0.5,  0.5,  1, 0,
         0.5,  0.5,  0.5,  1, 0,
         0.5,  0.5, -0.5,  1, 1,
         0.5, -0.5, -0.5,  0, 1,
         0.5, -0.5, -0.5,  0, 1,
         0.5, -0.5,  0.5,  0, 0,
         0.5,  0.5,  0.5,  1, 0,
        -0.5, -0.5, -0.5,  0, 1,
         0.5, -0.5, -0.5,  1, 1,
         0.5, -0.5,  0.5,  1, 0,
         0.5, -0.5,  0.5,  1, 0,
        -0.5, -0.5,  0.5,  0, 0,
        -0.5, -0.5, -0.5,  0, 1,
        -0.5,  0.5, -0.5,  0, 1,
         0.5,  0.5, -0.5,  1, 1,
         0.5,  0.5,  0.5,  1, 0,
         0.5,  0.5,  0.5,  1, 0,
        -0.5,  0.5,  0.5,  0, 0,
        -0.5,  0.5, -0.5,  0, 1
    };

    static const std::vector<GLfloat> floor_verts {
         // pos        tex coords
         5,  -0.5,  5,  2, 0,
        -5,  -0.5,  5,  0, 0,
        -5,  -0.5, -5,  0, 2,
         5,  -0.5,  5,  2, 0,
        -5,  -0.5, -5,  0, 2,
         5,  -0.5, -5,  2, 2
    };

    draw_objects(win, cube_verts, floor_verts, option);
}

// helper function to draw lighting objects
void draw_objects(GLFWwindow *win, const std::vector<GLfloat> &cube_verts,
        const std::vector<GLfloat> &floor_verts, const int option) {

    const Shader obj_shader = Shader {shad_path + "depth_test_01.vs",
        shad_path + "depth_test_01.frag"};

    std::vector<std::string> tex_imgs {{tex_path + "pattern4diffuseblack.jpg"},
        {tex_path + "metal.png"}};

    GLuint VAO_cube {}, VAO_floor {}, VBO_cube {}, VBO_floor {};

    // VAOs and VBOs
    std::vector<GLuint> VAO_vec {VAO_cube, VAO_floor};
    std::vector<GLuint> VBO_vec {VBO_cube, VBO_floor};

    const GLuint stride = 5;
    gen_objects(&VAO_vec, &VBO_vec);
    make_objects(VAO_vec[0], VBO_vec[0], cube_verts, stride);
    make_objects(VAO_vec[1], VBO_vec[1], floor_verts, stride);

    // texture
    const auto num_tex = tex_imgs.size();
    std::vector<GLuint> textures(num_tex);
    gen_textures(textures);
    for (size_t i {0}; i < num_tex; ++i)
        make_textures(textures[i], tex_imgs[i], {GL_REPEAT, GL_REPEAT},
                {GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR});

    const std::vector<size_t> num_verts {cube_verts.size() / stride,
        floor_verts.size() / stride};

    game_loop(win, VAO_vec, textures, num_verts, obj_shader, option);
}

// main loop for drawing light objects
void game_loop(GLFWwindow *win, const std::vector<GLuint> &VAO,
        const std::vector<GLuint> &tex_maps,
        const std::vector<size_t> &num_verts, const Shader &shad,
        const int option) {
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    const auto win_asp = float(win_w) / win_h;
    // shader for outlining the cubes
    const Shader color_shader {shad_path + "depth_test_01.vs",
        shad_path + "stencil_test_01.frag"};
    switch (option) {
        case 2:
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
            break;
        case 1:
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            break;
        default:
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
    // disable writing to the stencil buffer
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;
        glfwPollEvents();
        do_movement();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
        const auto view = main_cam.view_matrix();
        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);
        // drawing floor
        glStencilMask(0x00);
        draw_object(shad, VAO[1], tex_maps[1], view, proj, glm::mat4{},
                num_verts[1]);
        // drawing cubes: 1st pass
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        draw_object(shad, VAO[0], tex_maps[0], view, proj,
                glm::translate(glm::mat4{}, glm::vec3{-1, 0, -1}),
                num_verts[0]);
        draw_object(shad, VAO[0], tex_maps[0], view, proj,
                glm::translate(glm::mat4{}, glm::vec3{2, 0, 0}), num_verts[0]);
        // drawing cubes: 2nd pass
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        draw_object(color_shader, VAO[0], tex_maps[0], view, proj,
                glm::scale(glm::translate(glm::mat4{}, glm::vec3{-1, 0, -1}),
                    glm::vec3{1.1}), num_verts[0]);
        draw_object(color_shader, VAO[0], tex_maps[0], view, proj,
                glm::scale(glm::translate(glm::mat4{}, glm::vec3{2, 0, 0}),
                    glm::vec3{1.1}), num_verts[0]);
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        glfwSwapBuffers(win);
    }
}

// drawing an object
void draw_object(const Shader &shad, const GLuint VAO, const GLuint tex_map,
        const glm::mat4 &view, const glm::mat4 &proj, const glm::mat4 &mod,
        const GLuint num_verts) {
    shad.use();
    const auto idx = shad.id();

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, tex_map);

    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
            glm::value_ptr(mod));


    glDrawArrays(GL_TRIANGLES, 0, num_verts);

    glBindVertexArray(0);
}

