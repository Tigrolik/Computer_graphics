#include "shader.h"
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using namespace std;

void key_callback(GLFWwindow *window, int key, int scancode, int action,
        int mode);

const GLuint WIDTH = 800, HEIGHT = 600;

// uniform value for fragment shader mixVal
GLfloat mixVal = 0.2f;

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGL",
            nullptr, nullptr);
    if (window == nullptr) {
        cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    Shader ourShader("textures.vs", "textures3.frag");

    GLfloat vertices[] = {
        // positions       // colors        // texture coords
        0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
    };
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
            GL_STATIC_DRAW);

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

    // --- Load and create a texture
    GLuint texture1, texture2;
    // === texture1 ===
    glGenTextures(1, &texture1);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set texture wrapping parameters
    // set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int img_w, img_h;
    unsigned char *img = SOIL_load_image("container.jpg", &img_w, &img_h,
            0, SOIL_LOAD_RGB);
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

    // === texture2 ===
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    img = SOIL_load_image("awesomeface.png", &img_w, &img_h, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, GL_RGB,
            GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(img);
    glBindTexture(GL_TEXTURE_2D, 0);

    // wireframe polygons
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // -- end set up the triangle

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // activate shader
        ourShader.Use();

        // bind textures using texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

        // set value for mixVal
        glUniform1f(glGetUniformLocation(ourShader.Program, "mixVal"), mixVal);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }
    // properly de-allocate all resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
        int mode) {
    (void)scancode; (void)mode;
    switch (key) {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_UP:
            if (action == GLFW_PRESS) {
                mixVal += 0.1f;
                if (mixVal > 1.0f) mixVal = 1.0f;
            }
            break;
        case GLFW_KEY_DOWN:
            // testing release option
            if (action == GLFW_RELEASE) {
                mixVal -= 0.1f;
                if (mixVal < 0.0f) mixVal = 0.0f;
            }
            break;
    }
}
