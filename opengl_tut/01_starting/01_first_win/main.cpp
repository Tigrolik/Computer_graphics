/*
 * Following tutorial on OpenGL:
 * http://learnopengl.com/#!Getting-started/Hello-Window
 *
 * This is a simple program to make a window in OpenGL
 * Using GLEW and GLFW libraries
 */

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/*
 * Function to set up a glfw window
 */
void init_glfw() {
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
}

/*
 * Create a window
 */
GLFWwindow* make_window(const GLuint w, const GLuint h) {
    // --- create a window object
    GLFWwindow *window = glfwCreateWindow(w, h, "Hello", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
    } else {
        glfwMakeContextCurrent(window);
    }
    return window;
}

/*
 * initialize GLEW (manages function pointers for OpenGL) before calling
 * any OpenGL functions
 */
bool init_glew() {
    glewExperimental = GL_TRUE; // ensure the use of modern techniques
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }
    return true;
}

/*
 * Inform OpenGL about the size of the rendering window
 */
void make_viewport(GLFWwindow *window) {
    // size of the rendering window
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
}

/*
 * Loop for keeping the window drawing stuff - calling it a game loop
 */
void game_loop(GLFWwindow *window) {
    // check if GLFW has been told to close
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // check for triggered events
        // --- Rendering happens here
        // Set the color, clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // swap the screen buffers
        glfwSwapBuffers(window);
    }
}

/*
 * Call this function whenever a key is pressed / released
 */
void key_callback(GLFWwindow *window, const int key, const int,
        const int action, const int) {
    // when the escape key is pressed set the WindowShouldClose to true
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main() {

    // setting up (init GLFW)
    init_glfw();

    // OpenGL window size
    static constexpr GLuint width {800}, height {600};

    // make a window
    GLFWwindow *win = make_window(width, height);

    // check if the window creation has been successful
    if (win == nullptr)
        return 1;

    // register the callback
    glfwSetKeyCallback(win, key_callback);

    // let GLEW know to use a modern approach to retrieving function pointers
    if (!init_glew())
        return 1;

    // viewport
    make_viewport(win);

    // simple game loop
    game_loop(win);

    // clean up and exit properly
    glfwTerminate();

    return 0;
}

