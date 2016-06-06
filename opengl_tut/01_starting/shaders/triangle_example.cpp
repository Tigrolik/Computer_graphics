#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action,
        int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"// the color variable has attribute position 1\n"
"layout (location = 1) in vec3 color;\n"
"out vec3 ourColor; // specify a color output to the fragment shader\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position, 1.0);\n"
"ourColor = color;\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
"// the input variable from the vertex shader (same name and type)\n"
"in vec3 ourColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(ourColor, 1.0f);\n"
"}\n\0";

const GLchar* fragmentShaderSource2 = "#version 330 core\n"
"out vec4 color;\n"
"uniform vec4 ourColor2; // set this var in the OpenGL code\n"
"void main()\n"
"{\n"
"color = ourColor2;\n"
"}\n\0";

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL",
            nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving
    // function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);


    // Build and compile our shader program
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check for compile time errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" <<
            infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader[2];
    GLuint shaderProgram[2];
    fragmentShader[0] = glCreateShader(GL_FRAGMENT_SHADER);
    fragmentShader[1] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader[0], 1, &fragmentShaderSource, nullptr);
    glShaderSource(fragmentShader[1], 1, &fragmentShaderSource2, nullptr);
    for (int i = 0; i < 2; ++i) {
        glCompileShader(fragmentShader[i]);
        // Check for compile time errors
        glGetShaderiv(fragmentShader[i], GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader[i], 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" <<
                infoLog << std::endl;
        }
        shaderProgram[i] = glCreateProgram();
        glAttachShader(shaderProgram[i], vertexShader);
        glAttachShader(shaderProgram[i], fragmentShader[i]);
        glLinkProgram(shaderProgram[i]);
        glGetProgramiv(shaderProgram[i], GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram[i], 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" <<
                infoLog << std::endl;
        }
        glDeleteShader(fragmentShader[i]);
    }
    glDeleteShader(vertexShader);

    // Set up vertex data (and buffer(s)) and attribute pointers
    // We add a new set of vertices to form a second triangle (a total of 6
    // vertices); the vertex attribute configuration remains the same (still
    // one 3-float position vector per vertex)
    GLfloat firstTriangle[] = { // added colors
        -0.9f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // Left
        -0.0f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // Right
        -0.45f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // Top
    };
    GLfloat secondTriangle[] = {
        0.0f, -0.5f, 0.0f,  // Left
        0.9f, -0.5f, 0.0f,  // Right
        0.45f, 0.5f, 0.0f   // Top
    };
    GLuint VBOs[2], VAOs[2];
    // We can also generate multiple VAOs or buffers at the same time
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);
    // ================================
    // First Triangle setup
    // ===============================
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firstTriangle), firstTriangle,
            GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
            (GLvoid*)0);	// Vertex attributes stay the same
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
            (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    // ================================
    // Second Triangle setup
    // ===============================
    glBindVertexArray(VAOs[1]);	// Note that we bind to a different VAO now
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);	// And a different VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondTriangle), secondTriangle,
            GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    // Because the vertex data is tightly packed we can also specify 0 as the
    // vertex attribute's stride to let OpenGL figure it out.
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLint vertexColorLocation = glGetUniformLocation(shaderProgram[1],
                "ourColor2");
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved
        // etc.) and call corresponding response functions
        glfwPollEvents();

       // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Activate shader (same shader for both triangles)
        glUseProgram(shaderProgram[0]);
        // Draw first triangle using the data from the first VAO
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // Then we draw the second triangle using the data from the second VAO
        // GLfloat timeValue = glfwGetTime();
        // GLfloat greenValue = (sin(timeValue) + 1) * 0.5;
        glUseProgram(shaderProgram[1]);
        glUniform4f(vertexColorLocation, (cos(glfwGetTime()) + 1) * 0.5,
                (sin(glfwGetTime()) + 1) * 0.5, 0.0f, 1.0f);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteProgram(shaderProgram[0]);
    glDeleteProgram(shaderProgram[1]);
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action,
        int mode)
{
    (void)scancode; (void)mode;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
