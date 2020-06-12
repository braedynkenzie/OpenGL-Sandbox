#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;
};

// Function declarations
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void processInput(GLFWwindow* window);
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
static unsigned int CompileShader(unsigned int type, const std::string& source);
static ShaderProgramSource ParseShader(const std::string& filepath);

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Clamp framerate 
    GLCall(glfwSwapInterval(1));

    // Use GLEW to initialize all our modern OpenGL declarations
    if (glewInit() != GLEW_OK)
        std::cout << "ERROR: glewInit() did not return GLEW_OK" << std::endl;

    // Check OpenGL version
    std::cout << glGetString(GL_VERSION) << std::endl;

    // Callback functions:
    //
    // Window being resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    {
        // Create vertice positions
        float positions[] = {
            -0.5, -0.5,
             0.5,  0.5,
            -0.5,  0.5,
             0.5,  -0.5
        };

        unsigned int indices[]{
            0, 1, 2,
            3, 0, 1
        };

        // Create Vertex Array Object
        unsigned int VAO;
        GLCall(glGenVertexArrays(1, &VAO));
        GLCall(glBindVertexArray(VAO));

        // Create vertex buffer
        VertexBuffer VB(positions, 4 * 2 * sizeof(float));

        // Create and enable vertex attribute pointer
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

        // Create index buffer
        unsigned int EBO;
        IndexBuffer IB(indices, 6);

        // Shader programs
        ShaderProgramSource shaderSource = ParseShader("res/shaders/Basic.shader");
        // Debugging
        std::cout << "VERTEX SHADER SOURCE" << std::endl;
        std::cout << shaderSource.FragmentSource << std::endl;
        std::cout << "FRAGMENT SHADER SOURCE" << std::endl;
        std::cout << shaderSource.FragmentSource << std::endl;
        unsigned int shaderProgram = CreateShader(shaderSource.VertexSource, shaderSource.FragmentSource);

        // Unbind all buffers and vertex arrays
        GLCall(glBindVertexArray(0));
        GLCall(glUseProgram(0));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* User key input processing */
            processInput(window);

            /* Render here */
            GLCall(glClear(GL_COLOR_BUFFER_BIT));

            // Bind shader
            GLCall(glUseProgram(shaderProgram));

            // Set uniforms
            GLCall(int uniformLocation = glGetUniformLocation(shaderProgram, "u_Color"));
            GLCall(glUniform4f(uniformLocation, 0.2f, sin(glfwGetTime()) + 1.0f / 2.0f, 0.8f, 1.0f));

            // Bind VAO
            GLCall(glBindVertexArray(VAO));

            // Draw from element buffer
            GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

        // Release resources on termination
        GLCall(glDeleteProgram(shaderProgram));
    }
    glfwTerminate();
    return 0;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    GLCall(unsigned int shaderProgram = glCreateProgram());
    // Compile each shader individually
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    // Link both shaders to shaderProgram
    GLCall(glAttachShader(shaderProgram, vs));
    GLCall(glAttachShader(shaderProgram, fs));
    GLCall(glLinkProgram(shaderProgram));
    GLCall(glValidateProgram(shaderProgram));
    // Delete intermediate values
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));
    return shaderProgram;
}

unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* shader_src = source.c_str();
    glShaderSource(id, 1, &shader_src, nullptr);
    glCompileShader(id);

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE) {
        // Error handling
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*) alloca(length * sizeof(char)); // dynamic stack allocation
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "[ERROR]: " << (type == GL_VERTEX_SHADER ? "Vertex " : "Fragment ") << "shader compile failed." << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    // Read in a single file which contains both the vertex and fragment shader source codes
    std::ifstream stream(filepath);
    enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1 };
    ShaderType currentType = ShaderType::NONE;
    std::string currentLine;
    std::stringstream stringStream[2];
    while (getline(stream, currentLine)) 
    {
        if (currentLine.find("#shader") != std::string::npos)
        {
            // Update the shader type
            if (currentLine.find("vertex") != std::string::npos)
            {
                // Set mode to vertex shader
                currentType = ShaderType::VERTEX;
            }
            else if (currentLine.find("fragment") != std::string::npos)
            {
                // Set mode to fragment shader
                currentType = ShaderType::FRAGMENT;
            }
        }
        else 
        {
            // Append current line to proper string stream
            stringStream[(int)currentType] << currentLine << "\n";
        }
    }
    return ShaderProgramSource{ stringStream[0].str(), stringStream[1].str() };
}

void processInput(GLFWwindow* window)
{
    // Escape key to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle between wireframe/fully shaded modes
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}