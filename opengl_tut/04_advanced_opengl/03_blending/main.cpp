/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Advanced-OpenGL/Blending
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
// generate VAO, VBO, EBO...
void gen_objects(std::vector<GLuint>*, std::vector<GLuint>*);
// bind the objects with vertex data
void make_objects(const GLuint, const GLuint, const std::vector<GLfloat>&,
        const bool, const int, const GLuint);

// loading textures
GLuint load_texture(const std::string&, const GLboolean = false);

// drawing objects
void blend_test(GLFWwindow*, const int = 0);
void draw_object(const Shader&, const GLuint, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::mat4&, const GLuint);
void game_loop(GLFWwindow*, const std::vector<GLuint>&,
        const std::vector<GLuint>&, const std::vector<size_t>&,
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
        "This program demonstrates various blending options:\n" <<
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
        switch (inp_char - '0') {
            case 3:
                blend_test(win, 3);
                break;
            case 2:
                blend_test(win, 2);
                break;
            case 1:
                blend_test(win, 1);
                break;
            case 0:
            default:
                blend_test(win, 0);
        }
    } else {
        std::cerr << "Wrong input: drawing default scene\n";
        blend_test(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tcubes with grass without alpha blending (default)\n" <<
        "1:\tcubes and grass (alpha blending on)\n" <<
        "2:\tcubes and windows (not ordered, occlusions appear)\n" <<
        "3:\tcubes and windows (ordered)\n";
    blend_test(win, 0);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Blending", nullptr, nullptr);
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

    // enable depth and blend testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

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
        const std::vector<GLfloat> &vertices, const GLuint stride) {
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


// get cube object vertices
std::vector<GLfloat> cube_vertices() {
    return std::vector<GLfloat> {
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
}

// get floor object vertices
std::vector<GLfloat> floor_vertices() {
    return std::vector<GLfloat> {
        // pos        tex coords
         5, -0.5,  5,  2, 0,
        -5, -0.5,  5,  0, 0,
        -5, -0.5, -5,  0, 2,

         5, -0.5,  5,  2, 0,
        -5, -0.5, -5,  0, 2,
         5, -0.5, -5,  2, 2
    };
}

// get grass object vertices
std::vector<GLfloat> blend_vertices() {
    return std::vector<GLfloat> {
        // pos      tex coords
        0,  0.5, 0,  0, 0,
        0, -0.5, 0,  0, 1,
        1, -0.5, 0,  1, 1,

        0,  0.5, 0,  0, 0,
        1, -0.5, 0,  1, 1,
        1,  0.5, 0,  1, 0
    };
}

// get coordinates of cubes objects
std::vector<glm::vec3> cubes_positions() {
    return {
        glm::vec3{-1, 0, -1},
        glm::vec3{ 2, 0,  0}
    };
}

// get coordinates of grass objects
std::vector<glm::vec3> blend_positions() {
    return {
        glm::vec3{-1.5, 0, -0.48},
        glm::vec3{ 1.5, 0,  0.51},
        glm::vec3{   0, 0,   0.7},
        glm::vec3{-0.3, 0,  -2.3},
        glm::vec3{ 0.5, 0,  -0.6}
    };
}

// draw boxes on a floor
void blend_test(GLFWwindow *win, const int option) {
    static const std::vector<std::vector<GLfloat>> verts {
        cube_vertices(), floor_vertices(), blend_vertices()};

    // keep VAOs and VBOs for our objects in vectors
    static const auto num_obj = verts.size();
    std::vector<GLuint> VAO_vec (num_obj);
    std::vector<GLuint> VBO_vec (num_obj);

    const GLuint stride {5};
    gen_objects(&VAO_vec, &VBO_vec);
    for (std::size_t i {0}; i < num_obj; ++i)
        make_objects(VAO_vec[i], VBO_vec[i], verts[i], stride);

    // loading and mapping textures
    static std::vector<std::string> tex_imgs {tex_path +
        "pattern4diffuseblack.jpg", tex_path + "metal.png", tex_path +
            "grass.png"};
    if (option > 1) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        tex_imgs[2] = tex_path + "blending_transparent_window.png";
    }

    const auto num_tex = tex_imgs.size();
    std::vector<GLuint> textures(num_tex);
    for (std::size_t i {0}; i < num_tex - 1; ++i)
        textures[i] = load_texture(tex_imgs[i]);
    // grass object requires using alpha blending
    textures[num_tex - 1] = load_texture(tex_imgs[num_tex - 1], true);

    static Shader obj_shader;
    switch (option) {
        case 1:
            obj_shader = Shader {shad_path + "depth_test_01.vs",
                shad_path + "blend_test_01.frag"};
            break;
        case 0:
        default:
            obj_shader = Shader {shad_path + "depth_test_01.vs",
                shad_path + "depth_test_01.frag"};
    }

    game_loop(win, VAO_vec, textures, {verts[0].size() / stride,
            verts[1].size() / stride, verts[2].size() / stride}, obj_shader,
            option);
}

// main loop for drawing light objects
void game_loop(GLFWwindow *win, const std::vector<GLuint> &VAO,
        const std::vector<GLuint> &tex_maps,
        const std::vector<size_t> &num_verts, const Shader &shad,
        const int option) {
    const auto win_asp = window_aspect_ratio(win);

    static auto blend_pos = blend_positions();
    static std::vector<std::vector<glm::vec3>> poses {cubes_positions(),
        {glm::vec3{0, 0, 0}}, blend_pos};

    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;

        glfwPollEvents();
        do_movement();
        glClearColor(0.15, 0.15, 0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto view = main_cam.view_matrix();
        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);

        if (option == 3) { // sorting the positions of the objects
            std::map<float, glm::vec3> sorted_pos;
            for (size_t i {0}; i < blend_pos.size(); ++i)
                sorted_pos[glm::length(main_cam.pos() - blend_pos[i])] =
                    blend_pos[i];
            blend_pos.clear();
            // insert into the vector in the reversed order
            for (auto it = sorted_pos.rbegin(); it != sorted_pos.rend(); ++it)
                blend_pos.push_back(it->second);
            poses[2] = blend_pos;
        }

        for (size_t i {0}; i < num_verts.size(); ++i)
            for (size_t j {0}; j < poses[i].size(); ++j)
                draw_object(shad, VAO[i], tex_maps[i], view, proj,
                        glm::translate(glm::mat4{}, poses[i][j]),
                        num_verts[i]);

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

