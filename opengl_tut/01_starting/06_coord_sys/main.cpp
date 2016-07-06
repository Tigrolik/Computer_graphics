/*
 * Following tutorial on OpenGL (textures):
 * http://learnopengl.com/#!Getting-started/Coordinate-Systems
 *
 * Working with coordinate systems, drawing 3D boxes and applying
 * transformations to them
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

// path to the folder where we keep shaders and textures
static const std::string shad_path {"../../shaders/"};
static const std::string tex_path {"../../images/"};

/*
 * Functions declarations
 */
// initialize stuff
GLFWwindow* init(const GLuint, const GLuint);
// callback function
void key_callback(GLFWwindow*, const int, const int, const int, const int);
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
void lying_container(GLFWwindow*, const int = 0);
void lying_container_loop(GLFWwindow*, const GLuint, const Shader&,
        const std::vector<GLuint>&, const std::vector<std::string>&);
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

// here goes the main()
int main(int argc, char *argv[]) try {
    static constexpr GLuint width {800}, height {600};
    GLFWwindow *win = init(width, height);

    static constexpr char num_options {'2'};
    if (argc > 1) {
        const std::string s {argv[1]};
        const char inp_char {s[0]};
        if (s.length() == 1 && inp_char >= '0' && inp_char <= num_options) {
            switch (inp_char - '0') {
                case 1:
                    rotating_cube(win);
                    break;
                case 2:
                    rotating_cube(win, 2);
                    break;
                case 0:
                default:
                    lying_container(win);
            }
        } else {
            std::cerr << "Wrong input: drawing default rotating box\n";
            lying_container(win);
        }
    } else {
        std::cout << "Note: the program can be run as follows:\n" <<
            argv[0] << " int_param, where int_param is:\n" <<
            "0:\t\"lying\" box (default)\n" <<
            "1:\trotating box\n" <<
            "2:\trotating boxes (rotating every 3rd box)\n";
        lying_container(win);
    }

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
    GLFWwindow *win = glfwCreateWindow(w, h, "Coordinate systems", nullptr,
            nullptr);
    if (win == nullptr)
        throw std::runtime_error {"Failed to create GLFW window"};

    glfwMakeContextCurrent(win);

    // let GLEW know to use a modern approach to retrieving function pointers
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error {"Failed to initialize GLEW"};

    // register the callback
    glfwSetKeyCallback(win, key_callback);

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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GL_TRUE);
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

// draw a "lying" container with a smiley
void lying_container(GLFWwindow *win, const int) {
    // define vertices and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // texture coords
        0.5,  0.5, 0,    1, 1, // top-right
        0.5, -0.5, 0,    1, 0, // bottom-right
       -0.5, -0.5, 0,    0, 0, // bottom-left
       -0.5,  0.5, 0,    0, 1, // top-left
    };
    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

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

    lying_container_loop(win, VAO, shad, textures, frag_uni_tex);

    // cleaning up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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
    if (option == 2)
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
    else
        cubes_pos = { glm::vec3{ 0.0,  0.0,  0.0} };

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
        const std::vector<glm::vec3> &cubes_pos, const int) {
    // get window size
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);
    // view matrix
    const glm::mat4 view = glm::translate(glm::mat4{}, glm::vec3(0, 0, -2));
    // use window size for the projection matrix
    const auto proj = glm::perspective(glm::radians(60.0f),
            float(win_w) / win_h, 0.1f, 100.0f);
    const auto model_loc = glGetUniformLocation(shad.id(), "model");
    const auto view_loc  = glGetUniformLocation(shad.id(), "view");
    const auto proj_loc  = glGetUniformLocation(shad.id(), "proj");
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        glClearColor(0.6, 0.7, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // bind textures using texture units
        for (size_t i {0}; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glUniform1i(glGetUniformLocation(shad.id(),
                        tex_frag_uni[i].c_str()), i);
        }
        // activate shader
        shad.use();
        // set matrices
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
        // model matrix
        glBindVertexArray(VAO);
        glm::mat4 model;
        for (GLuint i = 0; i < cubes_pos.size(); ++i) {
            model = glm::translate(glm::mat4{}, cubes_pos[i]);
            model = glm::rotate(model, glm::radians(float(glfwGetTime()) * 50 +
                        20 * i) * (i % 3 == 0), glm::vec3{1, 0.3, 0.5});
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0 , 36);
        }
        glBindVertexArray(0);
        glfwSwapBuffers(win);
    }
}

// game loop for "lying" container
void lying_container_loop(GLFWwindow *win, const GLuint VAO, const Shader &shad,
        const std::vector<GLuint> &textures,
        const std::vector<std::string> &tex_frag_uni) {
    int win_w, win_h;
    glfwGetFramebufferSize(win, &win_w, &win_h);

    // define matrices
    const auto model = glm::rotate(glm::mat4{}, glm::radians(-55.0f),
            glm::vec3(1, 0, 0));
    const auto view = glm::translate(glm::mat4{}, glm::vec3{0, 0, -3});
    const auto proj = glm::perspective(45.0f, float(win_w) / win_h,
            0.1f, 100.0f);

    const auto model_loc = glGetUniformLocation(shad.id(), "model");
    const auto view_loc  = glGetUniformLocation(shad.id(), "view");
    const auto proj_loc  = glGetUniformLocation(shad.id(), "proj");

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        glClearColor(0.5, 0.8, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // bind textures using texture units
        for (size_t i {0}; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glUniform1i(glGetUniformLocation(shad.id(),
                        tex_frag_uni[i].c_str()), i);
        }
        // activate shader
        shad.use();
        // set matrices
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glfwSwapBuffers(win);
    }
}

