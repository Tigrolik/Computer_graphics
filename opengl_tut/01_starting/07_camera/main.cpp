/*
 * Following tutorial on OpenGL (textures):
 * http://learnopengl.com/#!Getting-started/Camera
 *
 * Experimenting with view matrix. Enabling user interaction with "camera" via
 * keyboard and mouse. Setting up "camera" view using Euler pitch * and yaw
 * angles. Making Camera class.
 *
 * Note: decided to add key Q for closing the main window (quitting), so now it
 * closes if a user presses either escape or Q button.
 */

#include <iostream>
#include <vector>
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
static Camera main_cam {glm::vec3{0, 0, 3}};

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
void gen_objects(GLuint*, GLuint*, GLuint*);
// bind the objects with vertex data
void make_objects(const GLuint, const GLuint, const GLuint,
        const std::vector<GLfloat>&, const std::vector<GLuint>&);

// init textures
void gen_textures(std::vector<GLuint>&);
void make_textures(const GLuint, const std::string&,
        const GLenum, const GLenum);

// drawing rotating and scaling containers
void rotating_cube(GLFWwindow*, const int = 0);
void rotating_cube_draw(GLFWwindow*, const std::vector<GLfloat>&,
        const std::vector<GLuint>&, const int);
void rotating_cube_loop(GLFWwindow*, const GLuint, const Shader&,
        const std::vector<GLuint>&, const std::vector<std::string>&,
        const std::vector<glm::vec3>&, const int);

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
    if (s.length() == 1 && inp_char >= '0' && inp_char <= num_options) {
        switch (inp_char - '0') {
            case 1:
                rotating_cube(win, 1);
                break;
            case 2:
                rotating_cube(win, 2);
                break;
            case 3:
                rotating_cube(win, 3);
                break;
            case 0:
            default:
                rotating_cube(win);
        }
    } else {
        std::cerr << "Wrong input: drawing default rotating cube\n";
        rotating_cube(win);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\trotating cube (default)\n" <<
        "1:\tcubes rotating on a \"sphere\"\n" <<
        "2:\tcamera moving with keys (WASD or arrow keys) and mouse" <<
        " (left-right and up-down movement)\n" <<
        "3:\tadded zooming (scrolling the mouse wheel, can be buggy...)\n";
    rotating_cube(win);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Triangle", nullptr, nullptr);
    if (win == nullptr)
        throw std::runtime_error {"Failed to create GLFW window"};

    glfwMakeContextCurrent(win);

    // let GLEW know to use a modern approach to retrieving function pointers
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error {"Failed to initialize GLEW"};

    // register keyboard callback
    glfwSetKeyCallback(win, key_callback);
    // set up mouse callback
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(win, mouse_callback);
    // scrolling
    glfwSetScrollCallback(win, scroll_callback);

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
 * Generate objects like Vertex Array Objects, Vertex Buffer Object
 */
void gen_objects(GLuint *VAO, GLuint *VBO, GLuint *EBO) {
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);
}

// exploiting std::vector to initialize several textures
void gen_textures(std::vector<GLuint> &tex) {
    for (auto &x: tex)
        glGenTextures(1, &x);
}

/*
 * Binding objects with vertex data.
 */
void make_objects(const GLuint VAO, const GLuint VBO, const GLuint EBO,
        const std::vector<GLfloat> &vertices,
        const std::vector<GLuint> &indices) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_of_elements(vertices), vertices.data(),
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_of_elements(indices),
            indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
            (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
            (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

/*
 * Binding textures with the image data
 */
void make_textures(const GLuint tex, const std::string& img_fn,
        const GLenum wrap, const GLenum filter) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    int img_w, img_h;
    unsigned char *img = SOIL_load_image(img_fn.c_str(), &img_w, &img_h, 0,
            SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, GL_RGB,
            GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(img);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// draw a rotating cube with a smiley
void rotating_cube(GLFWwindow *win, const int option) {
    // define vertices and indices for the cube
    static const std::vector<GLfloat> vertices {
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
    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    rotating_cube_draw(win, vertices, indices, option);
}

// helper function to initialize parameters for drawing a rotating cube
void rotating_cube_draw(GLFWwindow *win, const std::vector<GLfloat> &vertices,
        const std::vector<GLuint> &indices, const int option) {
    const Shader shad {shad_path + "transform_cont2.vs",
        shad_path + "transform_cont2.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg",
        tex_path + "awesomeface.png"};
    const std::vector<std::string> frag_uni_tex {"in_tex1", "in_tex2"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    // generate and build objects
    GLuint VAO, VBO, EBO;
    gen_objects(&VAO, &VBO, &EBO);
    make_objects(VAO, VBO, EBO, vertices, indices);

    // Load and create textures
    const auto num_imgs = tex_imgs.size();
    std::vector<GLuint> textures(num_imgs);
    gen_textures(textures);

    for (size_t i {0}; i < num_imgs; ++i)
        make_textures(textures[i], tex_imgs[i], GL_REPEAT, GL_LINEAR);

    // cubes positions
    std::vector<glm::vec3> cubes_pos;
    if (option == 0)
        cubes_pos = { glm::vec3{ 0.0,  0.0,  0.0} };
    else
        cubes_pos = {
            glm::vec3{ 0.0,  0.0,  0.0},
            glm::vec3{ 2.0,  5.0, -15.0},
            glm::vec3{-1.5, -2.2, -2.5},
            glm::vec3{-3.8, -2.0, -12.3},
            glm::vec3{ 2.4, -0.4, -3.5},
            glm::vec3{-1.7,  3.0, -7.5},
            glm::vec3{ 1.3, -2.0, -2.5},
            glm::vec3{ 1.5,  2.0, -2.5},
            glm::vec3{ 1.5,  0.2, -1.5},
            glm::vec3{-1.3,  1.0, -1.5}
        };

    rotating_cube_loop(win, VAO, shad, textures, frag_uni_tex, cubes_pos,
            option);

    // cleaning up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// implementing the drawing (game) loop for a rotating cube
void rotating_cube_loop(GLFWwindow *win, const GLuint VAO, const Shader &shad,
        const std::vector<GLuint> &textures,
        const std::vector<std::string> &tex_frag_uni,
        const std::vector<glm::vec3> &cubes_pos, const int option) {
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h); // get window size
    // use window size for the projection matrix
    glm::mat4 proj;
    if (option < 4)
        proj = glm::perspective(glm::radians(60.0f), float(win_w) / win_h,
                0.1f, 100.0f);
    const auto model_loc = glGetUniformLocation(shad.id(), "model");
    const auto view_loc  = glGetUniformLocation(shad.id(), "view");
    const auto proj_loc  = glGetUniformLocation(shad.id(), "proj");
    const GLfloat view_radius = 10;
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        const auto curr_time = glfwGetTime();
        delta_frame_time = curr_time - last_frame_time;
        last_frame_time  = curr_time;
        glm::mat4 view; // lookat view matrix
        if (option >= 2) { // a lot of crutches...
            do_movement();
            view = main_cam.view_matrix();
            if (option == 3)
                proj = glm::perspective(main_cam.zoom(), float(win_w) / win_h,
                        0.1f, 100.0f);
        } else {
            const auto cam_x = sin(curr_time) * view_radius;
            const auto cam_z = cos(curr_time) * view_radius;
            view = glm::lookAt(glm::vec3{cam_x, 0, cam_z},
                    glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});
        }
        glClearColor(0.6, 0.7, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // bind textures using texture units
        for (size_t i {0}; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glUniform1i(glGetUniformLocation(shad.id(),
                        tex_frag_uni[i].c_str()), i);
        }
        shad.use(); // activate shader
        // set matrices
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
        // model matrix
        glBindVertexArray(VAO);
        glm::mat4 model;
        for (GLuint i = 0; i < cubes_pos.size(); ++i) {
            model = glm::translate(glm::mat4{}, cubes_pos[i]);
            model = glm::rotate(model, glm::radians(float(curr_time) * 50 +
                        20 * i) * (i % 3 == 0), glm::vec3{1, 0.3, 0.5});
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0 , 36);
        }
        glBindVertexArray(0);
        glfwSwapBuffers(win);
    }
}

