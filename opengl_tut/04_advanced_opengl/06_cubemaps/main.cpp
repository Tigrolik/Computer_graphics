/*
 * Following tutorial on OpenGL (materials):
 * http://learnopengl.com/#!Advanced-OpenGL/Cubemaps
 *
 * Introduce the use of cubemaps, displaying different kinds of skyboxes as well
 * as reflection and refraction effects applied to either a box or model.
 *
 * Also changed the moving in the Camera class, now left-right movement happens
 * not along the axis but proper sideways, depending on where the camera looks
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

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

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
// generate VAO, VBO, EBO...
void gen_objects(std::vector<GLuint>*, std::vector<GLuint>*);
// bind the objects with vertex data
void make_objects(const GLuint, const GLuint, const std::vector<GLfloat>&,
        const bool, const int, const GLuint, const GLuint, const GLuint,
        const GLboolean = false);
// loading textures
GLuint load_texture(const std::string&, const GLboolean = false);

// load textures for a cubemap (skybox)
GLuint make_cubemap(const std::vector<std::string>&);
// get a shader depending on the option
Shader shader_for_object(const int);

// few function for retrieving vertices
std::vector<GLfloat> skybox_vertices();
std::vector<GLfloat> cube_vertices();
std::vector<GLfloat> cube_normal_vertices();

// drawing objects
void cubemap_test(GLFWwindow*, const int = 0);
void draw_object(const Shader&, const GLuint, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::mat4&, const GLuint = 36, const int = 0);
void game_loop(GLFWwindow*, const std::vector<GLuint>&,
        const std::vector<GLuint>&, const int = 0);
void draw_skybox(const Shader&, const GLuint, const GLuint, const glm::mat4&,
        const glm::mat4&);
void draw_model(Model&, const Shader&, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::mat4&, const int);

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
        "This program demonstrates various skyboxes and effects (reflection" <<
        "and refraction) applied to a box and model:\n" <<
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
    static constexpr char num_options {'6'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char < num_options) {
        switch (inp_char - '0') {
            case 5:
                cubemap_test(win, 5);
                break;
            case 4:
                cubemap_test(win, 4);
                break;
            case 3:
                cubemap_test(win, 3);
                break;
            case 2:
                cubemap_test(win, 2);
                break;
            case 1:
                cubemap_test(win, 1);
                break;
            case 0:
            default:
                cubemap_test(win, 0);
        }
    } else {
        std::cerr << "Wrong input: drawing default scene\n";
        cubemap_test(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tbox in a skybox with mountains and yellow light (default)\n" <<
        "1:\t\"mirror\" box in snowy mountains\n" <<
        "2:\t\"chrome plated\" model (suit) in skybox with lake\n" <<
        "3:\t\"glass\" box in a moonlight environment with a lake\n" <<
        "4:\t\"glass\" suit plus fiery sky and a mountain with light\n" <<
        "5:\tsuit with some parts reflecting colors in interstellar skybox\n";
    cubemap_test(win, 0);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Cubemaps", nullptr, nullptr);
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
        const GLuint offset, const GLuint num_att, const GLboolean is_skybox) {
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(vertices), vertices.data(),
            GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, offset, GL_FLOAT, GL_FALSE,
            stride * sizeof(GLfloat), (GLvoid*)0);

    if (!is_skybox) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, num_att, GL_FLOAT, GL_FALSE,
                stride * sizeof(GLfloat), (GLvoid*)(offset * sizeof(GLfloat)));
    }

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

GLuint make_cubemap(const std::vector<std::string>& texture_faces) {
    // generating a texture... typical steps
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

    // setting textures to the faces of the cubemap
    int img_w, img_h;
    //unsigned char* img;
    for (GLuint i {0}; i < texture_faces.size(); ++i) {
        unsigned char *img = SOIL_load_image(texture_faces[i].c_str(),
                &img_w, &img_h, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                img_w, img_h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex_id;
}

// choose a shader based on the option
Shader shader_for_object(const int option) {
    switch (option) {
        case 5:
            return Shader {shad_path + "cubemap_test_03.vs",
                shad_path + "cubemap_test_04.frag"};
        case 3:
        case 4:
            return Shader {shad_path + "cubemap_test_02.vs",
                shad_path + "cubemap_test_03.frag"};
        case 2:
        case 1:
            return Shader {shad_path + "cubemap_test_02.vs",
                shad_path + "cubemap_test_02.frag"};
        case 0:
        default:
            return Shader {shad_path + "depth_test_01.vs",
                shad_path + "depth_test_01.frag"};
    }
}

// skybox vertices
std::vector<GLfloat> skybox_vertices() {
    return std::vector<GLfloat> {
        -1,  1, -1,
        -1, -1, -1,
         1, -1, -1,
         1, -1, -1,
         1,  1, -1,
        -1,  1, -1,

        -1, -1,  1,
        -1, -1, -1,
        -1,  1, -1,
        -1,  1, -1,
        -1,  1,  1,
        -1, -1,  1,

         1, -1, -1,
         1, -1,  1,
         1,  1,  1,
         1,  1,  1,
         1,  1, -1,
         1, -1, -1,

        -1, -1,  1,
        -1,  1,  1,
         1,  1,  1,
         1,  1,  1,
         1, -1,  1,
        -1, -1,  1,

        -1,  1, -1,
         1,  1, -1,
         1,  1,  1,
         1,  1,  1,
        -1,  1,  1,
        -1,  1, -1,

        -1, -1, -1,
        -1, -1,  1,
         1, -1, -1,
         1, -1, -1,
        -1, -1,  1,
         1, -1,  1
    };
}

// get cube object vertices with normal vectors
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

// get cube object vertices
std::vector<GLfloat> cube_normal_vertices() {
    return std::vector<GLfloat> {
        // pos            // normals
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
}

// main testing function: loading object(s) and skybox
void cubemap_test(GLFWwindow *win, const int option) {
    static std::vector<std::vector<GLfloat>> verts {skybox_vertices(),
        cube_vertices()};
    static std::vector<GLuint> strides {3, 5};
    static std::vector<GLuint> offsets {3, 3};
    if (option == 1 || option == 3) {
        verts[1] = cube_normal_vertices();
        strides[1] = 6;
    }

    static const auto num_obj = verts.size();
    if (num_obj != strides.size() || num_obj != offsets.size())
        throw std::runtime_error
        {"Vectors of vertices mismatch vectors of strides and offsets"};
    // keep VAOs and VBOs for our objects in vectors
    std::vector<GLuint> VAO_vec (num_obj);
    std::vector<GLuint> VBO_vec (num_obj);

    gen_objects(&VAO_vec, &VBO_vec);
    // skybox and container
    make_objects(VAO_vec[0], VBO_vec[0], verts[0], strides[0], offsets[0], 3,
                true);
    if (option > 0) // crutches...
        make_objects(VAO_vec[1], VBO_vec[1], verts[1], strides[1], offsets[1],
                3, false);
    else
        make_objects(VAO_vec[1], VBO_vec[1], verts[1], strides[1], offsets[1],
                2, false);

    // load a cubemap
    //const std::string skybox_path {tex_path + "skybox_01/"};
    const std::string skybox_path {tex_path + "skybox_0" +
        std::to_string(option + 1) + '/'};
    static const std::vector<std::string> tex_faces {skybox_path + "right.jpg",
        skybox_path + "left.jpg", skybox_path + "top.jpg", skybox_path +
            "bottom.jpg", skybox_path + "back.jpg", skybox_path + "front.jpg"};
    const auto tex_cubemap = make_cubemap(tex_faces);
    // loading container
    auto tex_object = load_texture(tex_path + "container.jpg");
    if (option > 0)
        tex_object = tex_cubemap;

    game_loop(win, VAO_vec, {tex_cubemap, tex_object}, option);
}

// main loop function
void game_loop(GLFWwindow *win, const std::vector<GLuint> &VAO,
        const std::vector<GLuint> &tex_maps, const int option) {
    static const auto obj_shader = shader_for_object(option);
    static const Shader skybox_shader {shad_path + "cubemap_test_01.vs",
        shad_path + "cubemap_test_01.frag"};
    Model m {model_path + "crysis_nanosuit_refl/nanosuit.obj"};

    const auto win_asp = window_aspect_ratio(win);
    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;

        glfwPollEvents();
        do_movement();

        // Bind to framebuffer and draw to color texture
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f,
                100.0f);
        const auto view = main_cam.view_matrix();
        if (option == 0 || option == 1 || option == 3)
            draw_object(obj_shader, VAO[1], tex_maps[1], view, proj,
                    glm::mat4{}, 36, option);
        else
            draw_model(m, obj_shader, tex_maps[0], view, proj,
                    glm::scale(glm::translate(glm::mat4{},
                            glm::vec3{0, -1.75, 0}), glm::vec3{0.2}), option);
        // avoid translation for the skybox
        draw_skybox(skybox_shader, VAO[0], tex_maps[0],
                glm::mat4{glm::mat3{main_cam.view_matrix()}}, proj);

        glfwSwapBuffers(win);
    }
}

// drawing an object
void draw_object(const Shader &shad, const GLuint VAO, const GLuint tex_map,
        const glm::mat4 &view, const glm::mat4 &proj, const glm::mat4 &mod,
        const GLuint num_verts, const int option) {
    shad.use();
    const auto idx = shad.id();

    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
            glm::value_ptr(mod));
    if (option > 0)
        glUniform3f(glGetUniformLocation(idx, "cam_pos"), main_cam.pos().x,
                main_cam.pos().y, main_cam.pos().z);

    glBindVertexArray(VAO);
    if (option > 0)
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex_map);
    else
        glBindTexture(GL_TEXTURE_2D, tex_map);

    glDrawArrays(GL_TRIANGLES, 0, num_verts);

    glBindVertexArray(0);
}

// drawing the skybox
void draw_skybox(const Shader &shad, const GLuint VAO, const GLuint tex_map,
        const glm::mat4 &view, const glm::mat4 &proj) {
    glDepthFunc(GL_LEQUAL);

    shad.use();
    const auto idx = shad.id();

    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(idx, "tex_cubemap"), 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_map);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

// drawing a model
void draw_model(Model& m, const Shader &shad, const GLuint tex_map,
        const glm::mat4 &view, const glm::mat4 &proj, const glm::mat4 &mod,
        const int option) {
    shad.use();
    const auto idx = shad.id();

    glUniformMatrix4fv(glGetUniformLocation(idx, "view"), 1, GL_FALSE,
            glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(idx, "proj"), 1, GL_FALSE,
            glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(idx, "model"), 1, GL_FALSE,
            glm::value_ptr(mod));
    glUniform3f(glGetUniformLocation(idx, "cam_pos"), main_cam.pos().x,
            main_cam.pos().y, main_cam.pos().z);

    if (option == 5) {
        glActiveTexture(GL_TEXTURE3);
        glUniform1i(glGetUniformLocation(idx, "tex_cubemap"), 3);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_map);

    m.draw(shad);
}

