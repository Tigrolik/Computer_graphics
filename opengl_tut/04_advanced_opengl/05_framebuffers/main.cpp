/*
 * Following tutorial on OpenGL (materials):
 *
 * http://learnopengl.com/#!Advanced-OpenGL/Framebuffers
 *
 * Introduce the use of framebuffers and effects that can be applied to scenes
 * with the use of the framebuffers and convolution kernels
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
        const bool, const int, const GLuint, const GLuint);
// loading textures
GLuint load_texture(const std::string&, const GLboolean = false);

// create framebuffer
GLuint make_framebuffer();
//generate texture for the framebuffer
GLuint make_texture_fb(const int, const int);
GLuint make_texture_fb(GLFWwindow*);
// create a renderbuffer
GLuint make_renderbuffer(const int, const int);
GLuint make_renderbuffer(GLFWwindow*);
// choose a shader
Shader shader_for_framebuffer(const int);

// drawing objects
void fbuf_test(GLFWwindow*, const int = 0);
void draw_object(const Shader&, const GLuint, const GLuint, const glm::mat4&,
        const glm::mat4&, const glm::mat4&, const GLuint);
void draw_framebuffer(const Shader&, const GLuint, const GLuint, const GLuint);
void game_loop(GLFWwindow*, const std::vector<GLuint>&,
        const std::vector<GLuint>&, const std::vector<size_t>&, const int);

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
        "This program demonstrates various post-processing options " <<
        "involving framebuffers:\n" <<
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
    static constexpr char num_options {'7'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char < num_options) {
        switch (inp_char - '0') {
            case 6:
                fbuf_test(win, 6);
                break;
            case 5:
                fbuf_test(win, 5);
                break;
            case 4:
                fbuf_test(win, 4);
                break;
            case 3:
                fbuf_test(win, 3);
                break;
            case 2:
                fbuf_test(win, 2);
                break;
            case 1:
                fbuf_test(win, 1);
                break;
            case 0:
            default:
                fbuf_test(win, 0);
        }
    } else {
        std::cerr << "Wrong input: drawing default scene\n";
        fbuf_test(win, 0);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tcontainers on a metal floor (default)\n" <<
        "1:\tcolors (from the previous scene) inverted\n" <<
        "2:\tgrayscale scene\n" <<
        "3:\t\"sharpened\" scene\n" <<
        "4:\tblurred scene\n" <<
        "5:\tscene with \"edge detection\"\n" <<
        "6:\toriginal scene with a rear-view mirror\n";
    fbuf_test(win, 0);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Framebuffers", nullptr, nullptr);
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
        const GLuint offset) {
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_in_bytes(vertices), vertices.data(),
            GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, offset, GL_FLOAT, GL_FALSE,
            stride * sizeof(GLfloat), (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat),
            (GLvoid*)(offset * sizeof(GLfloat)));

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

/*
 * Create framebuffer
 */
GLuint make_framebuffer() {
    // create a framebuffer object
    GLuint fbo;
    glGenFramebuffers(1, &fbo);

    // bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    return fbo;
}

/*
 * Generate texture for the framebuffer
 */
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
    return tex_id;
}

// version with GLFWwindow as a parameter
GLuint make_texture_fb(GLFWwindow *win) {
    // get window size
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);

    return make_texture_fb(win_w, win_h);
}

/*
 * Create a renderbuffer
 */
GLuint make_renderbuffer(const int win_w, const int win_h) {
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, win_w, win_h);
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
GLuint make_renderbuffer(GLFWwindow *win) {
    // get window size
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    return make_renderbuffer(win_w, win_h);
}

// choose a shader based on the option
Shader shader_for_framebuffer(const int option) {
    switch (option) {
        case 5:
            return Shader {shad_path + "framebuffer_01.vs",
                shad_path + "framebuffer_05.frag"};
        case 4:
            return Shader {shad_path + "framebuffer_01.vs",
                shad_path + "framebuffer_04.frag"};
        case 3:
            return Shader {shad_path + "framebuffer_01.vs",
                shad_path + "framebuffer_03.frag"};
        case 2:
            return Shader {shad_path + "framebuffer_01.vs",
                shad_path + "framebuffer_02.frag"};
        case 1:
            return Shader {shad_path + "framebuffer_01.vs",
                shad_path + "framebuffer_01.frag"};
        case 0:
        default:
            return Shader {shad_path + "framebuffer_01.vs",
                shad_path + "depth_test_01.frag"};
    }
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

// get quad object vertices
std::vector<GLfloat> quad_vertices() {
    return std::vector<GLfloat> {
        // pos, tex coords
        -1,  1, 0, 1,
        -1, -1, 0, 0,
         1, -1, 1, 0,

        -1,  1, 0, 1,
         1, -1, 1, 0,
         1,  1, 1, 1
    };
}

// get quad object vertices
std::vector<GLfloat> mirror_quad_vertices() {
    return std::vector<GLfloat> {
        // pos      tex coords
        -0.3,   1, 1, 1,
        -0.3, 0.7, 1, 0,
         0.3, 0.7, 0, 0,

        -0.3,   1, 1, 1,
         0.3, 0.7, 0, 0,
         0.3,   1, 0, 1
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
void fbuf_test(GLFWwindow *win, const int option) {
    static const std::vector<std::vector<GLfloat>> verts {
        cube_vertices(), floor_vertices(), quad_vertices(),
            mirror_quad_vertices()};

    static const std::vector<GLuint> strides {5, 5, 4, 4};
    static const std::vector<GLuint> offsets {3, 3, 2, 2};

    static const auto num_obj = verts.size();
    if (num_obj != strides.size() || num_obj != offsets.size())
        throw std::runtime_error
        {"Vectors of vertices mismatch vectors of strides and offsets"};
    // keep VAOs and VBOs for our objects in vectors
    std::vector<GLuint> VAO_vec (num_obj);
    std::vector<GLuint> VBO_vec (num_obj);

    gen_objects(&VAO_vec, &VBO_vec);
    for (std::size_t i {0}; i < num_obj; ++i)
        make_objects(VAO_vec[i], VBO_vec[i], verts[i], strides[i], offsets[i]);

    // loading and mapping textures
    static std::vector<std::string> tex_imgs {tex_path +
        "container.jpg", tex_path + "metal.png"};

    const auto num_tex = tex_imgs.size();
    std::vector<GLuint> textures(num_tex);
    for (std::size_t i {0}; i < num_tex; ++i)
        textures[i] = load_texture(tex_imgs[i]);

    // put the framebuffer objects into the corresponding vectors
    VAO_vec.push_back(make_framebuffer());
    textures.push_back(make_texture_fb(win));
    VBO_vec.push_back(make_renderbuffer(win));

    game_loop(win, VAO_vec, textures, {verts[0].size() / strides[0],
            verts[1].size() / strides[1], verts[2].size() / strides[2],
            verts[3].size() / strides[3]}, option);
}

// main loop for drawing light objects
void game_loop(GLFWwindow *win, const std::vector<GLuint> &VAO,
        const std::vector<GLuint> &tex_maps,
        const std::vector<size_t> &num_verts, const int option) {
    const auto win_asp = window_aspect_ratio(win);

    const static Shader obj_shader {shad_path + "depth_test_01.vs",
        shad_path + "depth_test_01.frag"};
    const static Shader frame_shader {shader_for_framebuffer(option)};

    static const std::vector<glm::vec3> poses {cubes_positions()};

    while (!glfwWindowShouldClose(win)) {
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;
        glfwPollEvents();
        do_movement();

        // Bind to framebuffer and draw to color texture
        glBindFramebuffer(GL_FRAMEBUFFER, VAO[VAO.size() - 1]);
        glClearColor(0.15, 0.15, 0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 proj {}, view {};
        if (option == 6) {
            // making rear view mirror
            main_cam.rear_view(); // set the rear view
            view = main_cam.view_matrix();
            main_cam.rear_view(); // return the original view
            proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f, 100.0f);
            draw_object(obj_shader, VAO[1], tex_maps[1], view, proj,
                    glm::mat4{}, num_verts[1]);
            for (size_t i {0}; i < 2; ++i)
                draw_object(obj_shader, VAO[0], tex_maps[0], view, proj,
                        glm::translate(glm::mat4{}, poses[i]), num_verts[0]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0.15, 0.15, 0.15, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        view = main_cam.view_matrix();
        proj = glm::perspective(main_cam.zoom(), win_asp, 0.1f, 100.0f);
        // draw objects
        draw_object(obj_shader, VAO[1], tex_maps[1], view, proj, glm::mat4{},
                num_verts[1]);
        for (size_t i {0}; i < 2; ++i)
            draw_object(obj_shader, VAO[0], tex_maps[0], view, proj,
                    glm::translate(glm::mat4{}, poses[i]), num_verts[0]);

        // draw the framebuffer
        draw_framebuffer(frame_shader, VAO[option == 6 ? 3 : 2], tex_maps[2],
                num_verts[2]);

        glfwSwapBuffers(win);
    }

    glDeleteFramebuffers(1, &VAO[VAO.size() - 1]);
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

/*
 * Bind to the default framebuffer and draw the quad plane with attached
 * screen texture
 */
void draw_framebuffer(const Shader &shad, const GLuint VAO,
        const GLuint tex_map, const GLuint num_verts) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // clear all relevant buffers
    //glClearColor(1, 1, 1, 1);
    //glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    shad.use();

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, tex_map);
    glDrawArrays(GL_TRIANGLES, 0, num_verts);

    glBindVertexArray(0);

}

