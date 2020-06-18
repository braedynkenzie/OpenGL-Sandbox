#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw_gl3.h"

#include "tests\Test.h"
#include "tests\TestClearColour.h"
#include "tests\TestTexture2D.h"
#include "tests\TestFPSCamera.h"

// Function declarations
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void processInput(GLFWwindow* window);

// Global vars
static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

int main(void)
{
    GLFWwindow* window;

    /* Initialize glfw library */
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 540, "OpenGL sandbox", NULL, NULL);
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
        Renderer renderer;

        // ImGui initialization stuff
        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init(window, true);
        ImGui::StyleColorsDark();
        
        // Test framework
        test::Test* activeTest = nullptr;
        test::TestMenu* testMenu = new test::TestMenu(activeTest);
        activeTest = testMenu;
        // Init tests
        test::TestClearColour* clearColourTest  = new test::TestClearColour(window);
        test::TestTexture2D* texture2DTest      = new test::TestTexture2D(window);
        test::TestFPSCamera* cameraTest         = new test::TestFPSCamera(window);
        testMenu->RegisterTest<test::TestClearColour*>("Clear colour test", (test::TestClearColour*) clearColourTest);
        testMenu->RegisterTest<test::TestTexture2D*>("2D Texture test", (test::TestTexture2D*) texture2DTest);
        testMenu->RegisterTest<test::TestFPSCamera*>("First person camera test", (test::TestFPSCamera*) cameraTest);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* User key input processing */
            processInput(window);

            // Start each new frame by clearing
            GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
            renderer.Clear();

            ImGui_ImplGlfwGL3_NewFrame();
            if (activeTest)
            {
                activeTest->OnUpdate(0.0f);
                activeTest->OnRender();
                ImGui::Begin("Test directory");
                if (activeTest != testMenu && ImGui::Button("<-"))
                {
                    //delete activeTest;

                    // Testing
                    //delete testMenu;
                    //activeTest = nullptr;
                    //testMenu = new test::TestMenu(activeTest);
                    activeTest = testMenu;
                }
                activeTest->OnImGuiRender();
                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

        // TODO: debug
        // Release resources on termination
        /*if (activeTest != testMenu)
            delete testMenu;
        delete activeTest;*/
    }
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
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