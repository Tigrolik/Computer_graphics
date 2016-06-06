#include "shader.h"
#include <GLFW/glfw3.h>

using namespace std;

void key_callback(GLFWwindow *window, int key, int scancode, int action,
        int mode);

const GLuint WIDTH = 800, HEIGHT = 600;

int main()
{
    // initialize GLFW
    glfwInit();

    // --- configuring GLFW ---
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // explicitly declare the use of the core-profile: (result in "invalid
    // operation" errors when using old functionality
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // a user cannot resize the window
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // --- create a window object
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGL",
            nullptr, nullptr);
    if (window == nullptr) {
        cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // register the callback
    glfwSetKeyCallback(window, key_callback);

    // initialize GLEW (manages function pointers for OpenGL) before calling
    // any OpenGL functions
    glewExperimental = GL_TRUE; // ensure the use of modern techniques
    if (glewInit() != GLEW_OK) {
        cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    // size of the rendering window
    glViewport(0, 0, WIDTH, HEIGHT);

    // build and compile our shader program
    Shader ourShader("vert.vs", "frag.frag");

    // vertices for rect
    GLfloat vertices[] = {
        0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top-right
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
       -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
       -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f,  // top-left
    };

    GLuint VAO, VBO;

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // generate buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
            (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
            (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // --- simple game loop
    // check if GLFW has been told to close
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // check for triggered events

        // Rendering
        // Set the color, clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.Use();

        glBindVertexArray(VAO);
        glDrawArrays(GL_QUADS, 0, 4);
        glBindVertexArray(0);

        // swap the screen buffers
        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // clean up and exit properly
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
        int mode) {
    (void)scancode; (void)mode;
    // when the escape key is pressed set the WindowShouldClose to true
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
