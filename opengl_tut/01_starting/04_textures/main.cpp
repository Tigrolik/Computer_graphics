/*
 * Following tutorial on OpenGL (textures):
 * http://learnopengl.com/#!Getting-started/Textures
 *
 * This time we practice drawing images as textures. The textures are applied on
 * the rectangle shape that has been used before. Most changes happen in the
 * fragment shaders, thus drawing functions implementations look alike.
 */

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include "Shader.h"

// path to the folder where we keep shaders and textures
static const std::string shad_path {"../../shaders/"};
static const std::string tex_path {"../../images/"};
// this value changes while up-down arrow keys are pressed/released
static GLfloat mix_val {0.2};

/*
 * Functions declarations
 */
// initialize stuff
GLFWwindow* init(const GLuint, const GLuint);
// callback function
void key_callback(GLFWwindow*, const int, const int, const int, const int);
// drawing loop
void game_loop(GLFWwindow*, const GLuint, const Shader&,
        const std::vector<GLuint>&, const std::vector<std::string>&,
        const GLuint, const int = 0);
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

// drawing containers
void draw_container(GLFWwindow*);
void draw_disco_container(GLFWwindow*);
void draw_container_with_face(GLFWwindow*);
void draw_container_with_face_to_left(GLFWwindow*);
void draw_four_containers(GLFWwindow*);
void draw_center_pixels(GLFWwindow*);
void draw_container_face_mix(GLFWwindow*);
void draw_wall_triangle(GLFWwindow*);
// common container drawing function
void drawing_container(GLFWwindow*, const Shader&,
        const std::vector<std::string>&, const std::vector<std::string>&,
        const std::vector<GLfloat>&, const std::vector<GLuint>&,
        const std::vector<std::vector<GLenum>> = {{GL_REPEAT, GL_LINEAR},
        {GL_REPEAT, GL_LINEAR}}, const int = 0);

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
    static constexpr char num_options {'7'};
    const std::string s {inp};
    const char inp_char {s[0]};
    if (s.length() == 1 && inp_char >= '0' && inp_char <= num_options) {
        switch (inp_char - '0') {
            case 1:
                draw_disco_container(win);
                break;
            case 2:
                draw_container_with_face(win);
                break;
            case 3:
                draw_container_with_face_to_left(win);
                break;
            case 4:
                draw_four_containers(win);
                break;
            case 5:
                draw_center_pixels(win);
                break;
            case 6:
                draw_container_face_mix(win);
                break;
            case 7:
                draw_wall_triangle(win);
                break;
            case 0:
            default:
                draw_container(win);
        }
    } else {
        std::cerr << "Wrong input: drawing default box\n";
        draw_container(win);
    }
}

/*
 * Display a menu of possible actions
 */
void show_menu(GLFWwindow *win, const std::string &prog_name) {
    std::cout << "Note: the program can be run as follows:\n" <<
        prog_name << " int_param, where int_param is:\n" <<
        "0:\tbox (default)\n" <<
        "1:\t\"disco\" box\n" <<
        "2:\tbox with a smiley\n" <<
        "3:\tbox with smiley looking to the left\n" <<
        "4:\tfour boxes with smileys\n" <<
        "5:\tcenter box-smiley pixels\n" <<
        "6:\tbox-smiley (use up-down arrow keys)\n" <<
        "7:\ttriangle with a brick wall pattern\n";
    draw_container(win);
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
    GLFWwindow *win = glfwCreateWindow(w, h, "Textures", nullptr, nullptr);
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

    return win;
}

/*
 * Game loop - keep things drawing
 */
void game_loop(GLFWwindow *win, const GLuint VAO, const Shader &shad,
        const std::vector<GLuint> &tex, const std::vector<std::string> &samp,
        const GLuint num_nodes, const int option) {
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        glClearColor(0.41f, 0.75f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shad.use();

        // bind textures using texture units
        for (size_t i {0}; i < tex.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, tex[i]);
            glUniform1i(glGetUniformLocation(shad.id(), samp[i].c_str()), i);
        }
        // crutches...
        if (option)
            glUniform1f(glGetUniformLocation(shad.id(), "mval"), mix_val);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, num_nodes, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(win);
    }
}

/*
 * Call this function whenever a key is pressed / released
 */
void key_callback(GLFWwindow *win, const int key, const int,
        const int action, const int) {
    if (action == GLFW_PRESS)
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(win, GL_TRUE);
                break;
            case GLFW_KEY_UP:
                mix_val = std::min(mix_val + 0.1, 1.0);
                break;
        }
    else if (action == GLFW_RELEASE && key == GLFW_KEY_DOWN)
        mix_val = std::max(mix_val - 0.1, 0.0);
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

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
            (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
            (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // TexCoord attibute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
            (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

/*
 * Binding textures with the image data
 */
void make_textures(const GLuint tex, const std::string& img_fn,
        const GLenum wrap, const GLenum filter) {
    // bind texture
    glBindTexture(GL_TEXTURE_2D, tex);
    // set texture wrapping parameters
    // set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    // load image from a file, create texture and generate mipmaps
    int img_w, img_h;
    unsigned char *img = SOIL_load_image(img_fn.c_str(), &img_w, &img_h, 0,
            SOIL_LOAD_RGB);
    // start generating a texture, parameters:
    // 1) texture target
    // 2) mipmap level (0 = base level)
    // 3) format to store the texture
    // 4) and 5) texture size
    // 6) should be zero... (some legacy stuff)
    // 7) and 8) format and datatype of the source image
    // 9) actual image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, GL_RGB,
            GL_UNSIGNED_BYTE, img);
    // generate mipmaps automatically
    glGenerateMipmap(GL_TEXTURE_2D);
    // after generating the texture it is good to free image memory and unbind
    SOIL_free_image_data(img);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// drawing default box container
void draw_container(GLFWwindow *win) {
    // define vertices (pos, col, tex) and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // colors   // texture coords
        0.5,  0.5, 0,   1, 0, 0,   1, 1, // top-right
        0.5, -0.5, 0,   0, 1, 0,   1, 0, // bottom-right
       -0.5, -0.5, 0,   0, 0, 1,   0, 0, // bottom-left
       -0.5,  0.5, 0,   1, 1, 0,   0, 1, // top-left
    };

    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg"};
    const std::vector<std::string> frag_uni_tex {"in_texture"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices);
}

// apply little color play to the box
void draw_disco_container(GLFWwindow *win) {
    // define vertices (pos, col, tex) and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // colors   // texture coords
        0.5,  0.5, 0,   1, 0, 0,   1, 1, // top-right
        0.5, -0.5, 0,   0, 1, 0,   1, 0, // bottom-right
       -0.5, -0.5, 0,   0, 0, 1,   0, 0, // bottom-left
       -0.5,  0.5, 0,   1, 1, 0,   0, 1, // top-left
    };
    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container_disco.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg"};
    const std::vector<std::string> frag_uni_tex {"in_texture"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices);
}

// draw a smiley on top of the box
void draw_container_with_face(GLFWwindow *win) {
    // define vertices (pos, col, tex) and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // colors   // texture coords
        0.5,  0.5, 0,   1, 0, 0,   1, 1, // top-right
        0.5, -0.5, 0,   0, 1, 0,   1, 0, // bottom-right
       -0.5, -0.5, 0,   0, 0, 1,   0, 0, // bottom-left
       -0.5,  0.5, 0,   1, 1, 0,   0, 1, // top-left
    };
    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container_face.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg",
        tex_path + "awesomeface.png"};
    const std::vector<std::string> frag_uni_tex {"in_tex1", "in_tex2"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices);
}

// make the smiley look to the other direction
void draw_container_with_face_to_left(GLFWwindow *win) {
    // define vertices (pos, col, tex) and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // colors   // texture coords
        0.5,  0.5, 0,   1, 0, 0,   1, 1, // top-right
        0.5, -0.5, 0,   0, 1, 0,   1, 0, // bottom-right
       -0.5, -0.5, 0,   0, 0, 1,   0, 0, // bottom-left
       -0.5,  0.5, 0,   1, 1, 0,   0, 1, // top-left
    };
    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container_face_left.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg",
        tex_path + "awesomeface.png"};
    const std::vector<std::string> frag_uni_tex {"in_tex1", "in_tex2"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices);
}

// practicing drawing boxes and smileys in different ways
void draw_four_containers(GLFWwindow *win) {
    // define vertices (pos, col, tex) and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // colors   // texture coords
        0.5,  0.5, 0,   1, 0, 0,   2, 2, // top-right
        0.5, -0.5, 0,   0, 1, 0,   2, 0, // bottom-right
       -0.5, -0.5, 0,   0, 0, 1,   0, 0, // bottom-left
       -0.5,  0.5, 0,   1, 1, 0,   0, 2, // top-left
    };
    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container_face.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg",
        tex_path + "awesomeface.png"};
    const std::vector<std::string> frag_uni_tex {"in_tex1", "in_tex2"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices,
            {{GL_CLAMP_TO_EDGE, GL_NEAREST}, {GL_MIRRORED_REPEAT, GL_LINEAR}});
}

// focus on the center pixels
void draw_center_pixels(GLFWwindow *win) {
    // define vertices (pos, col, tex) and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // colors   // texture coords
        0.5,  0.5, 0,   1, 0, 0,   0.55, 0.55, // top-right
        0.5, -0.5, 0,   0, 1, 0,   0.55, 0.45, // bottom-right
       -0.5, -0.5, 0,   0, 0, 1,   0.45, 0.45, // bottom-left
       -0.5,  0.5, 0,   1, 1, 0,   0.45, 0.55, // top-left
    };

    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container_face.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg",
        tex_path + "awesomeface.png"};
    const std::vector<std::string> frag_uni_tex {"in_tex1", "in_tex2"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices);
}

// a user can change the mixing values by pressing up-down keys
void draw_container_face_mix(GLFWwindow *win) {
    // define vertices (pos, col, tex) and indices for the container
    static const std::vector<GLfloat> vertices {
        // positions   // colors   // texture coords
        0.5,  0.5, 0,   1, 0, 0,   1, 1, // top-right
        0.5, -0.5, 0,   0, 1, 0,   1, 0, // bottom-right
       -0.5, -0.5, 0,   0, 0, 1,   0, 0, // bottom-left
       -0.5,  0.5, 0,   1, 1, 0,   0, 1, // top-left
    };
    static const std::vector<GLuint> indices {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container_face_mix.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "container.jpg",
        tex_path + "awesomeface.png"};
    const std::vector<std::string> frag_uni_tex {"in_tex1", "in_tex2"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices,
            {{GL_REPEAT, GL_LINEAR}, {GL_REPEAT, GL_LINEAR}}, 1);
}

// draw a triangle with a wall pattern
void draw_wall_triangle(GLFWwindow *win) {
    static const std::vector<GLfloat> vertices {
        // positions    // colors   // texture coords
       -0.5, -0.5, 0,   1, 0, 0,   0.0, 0, // left
        0.5, -0.5, 0,   0, 1, 0,   1.0, 0, // top
        0.0,  0.5, 1,   0, 0, 1,   0.5, 1, // right
    };
    static const std::vector<GLuint> indices {
        0, 1, 2, // only one triangle
    };

    const Shader shad {shad_path + "container.vs",
        shad_path + "container.frag"};

    const std::vector<std::string> tex_imgs {tex_path + "wall.jpg"};
    const std::vector<std::string> frag_uni_tex {"in_texture"};
    assert(tex_imgs.size() == frag_uni_tex.size());

    // still using box drawing for a triangle :)
    drawing_container(win, shad, tex_imgs, frag_uni_tex, vertices, indices);
}

/*
 * General function for drawing a box: has a bunch of parameters, however, this
 * function is supposed to be only for implementation and not meant for the user
 * interface
 */
void drawing_container(GLFWwindow *win, const Shader &shad,
        const std::vector<std::string> &tex_imgs,
        const std::vector<std::string> &tex_frag_uni,
        const std::vector<GLfloat> &vertices,
        const std::vector<GLuint> &indices,
        const std::vector<std::vector<GLenum>> params,
        const int option) {
    const auto num_imgs = tex_imgs.size();
    assert(tex_frag_uni.size() == num_imgs);

    // generate and build objects
    GLuint VAO, VBO, EBO;
    gen_objects(&VAO, &VBO, &EBO);
    make_objects(VAO, VBO, EBO, vertices, indices);

    // Load and create textures
    std::vector<GLuint> textures(num_imgs);
    gen_textures(textures);

    for (size_t i {0}; i < num_imgs; ++i)
        make_textures(textures[i], tex_imgs[i], params[i][0], params[i][1]);

    game_loop(win, VAO, shad, textures, tex_frag_uni, indices.size(), option);

    // cleaning up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

