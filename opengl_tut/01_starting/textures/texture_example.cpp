#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

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

    // load a texture image with SOIL
    int img_w, img_h;
    unsigned char *img = SOIL_load_image("container.jpg", &img_w, &img_h,
            0, SOIL_LOAD_RGB);
    // cout << image[9] << endl;

    // generate a texture ID
    GLuint texture;
    glGenTextures(1, &texture);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture);
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

    // --- simple game loop
    // check if GLFW has been told to close
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // check for triggered events

        // Rendering
        // Set the color, clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // swap the screen buffers
        glfwSwapBuffers(window);
    }

    // clean up and exit properly
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
        int mode) {
    (void)scancode; (void)mode;
    cout << key << endl;
    // when the escape key is pressed set the WindowShouldClose to true
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
