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
#include "tests\TestPhongLighting.h"
#include "tests\TestModelLoading.h"
#include "tests\TestManualFramebuffers.h"
#include "tests\TestCubemapping.h"
#include "tests\TestGeometryShader.h"
#include "tests\TestTemplate.h"
#include "tests\TestInstancedRendering.h"
#include "tests\TestShadowMapping.h"
#include "tests\TestPointShadowMapping.h"
#include "tests\TestParallaxNormalMapping.h"
#include "tests\TestHDRBloom.h"
#include "tests\TestDeferredRendering.h"
#include "tests\TestSSAmbientOcclusion.h"
#include <Globals.h>

// Function declarations
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void processInput(GLFWwindow* window);

test::Test* activeTest;
test::TestMenu* testMenu;

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
    window = glfwCreateWindow(1920, 1080, "OpenGL sandbox", NULL, NULL);
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

        // Flip texture along y axis before loading
        stbi_set_flip_vertically_on_load(true);
        
        // Test framework
        activeTest = nullptr;
        testMenu = new test::TestMenu(activeTest, window);
        activeTest = testMenu;
        // Initialize test sandboxes
        test::TestClearColour* clearColourTest  = new test::TestClearColour(window);
        test::TestTexture2D* texture2DTest      = new test::TestTexture2D(window);
        test::TestFPSCamera* cameraTest = new test::TestFPSCamera(window);
        test::TestPhongLighting* phongLightingTest = new test::TestPhongLighting(window);
        test::TestModelLoading* modelLoadingTest = new test::TestModelLoading(window);
        test::TestManualFramebuffer* framebufferTest = new test::TestManualFramebuffer(window);
        test::TestCubemapping* cubemappingTest = new test::TestCubemapping(window);
        test::TestGeometryShader* geometryShaderTest = new test::TestGeometryShader(window);
        test::TestInstancedRendering* instancedRenderingTest = new test::TestInstancedRendering(window);
        test::TestShadowMapping* shadowMappingTest = new test::TestShadowMapping(window);
        test::TestPointShadowMapping* pointShadowMappingTest = new test::TestPointShadowMapping(window);
        test::TestParallaxNormalMapping* parallaxNormalTest = new test::TestParallaxNormalMapping(window);
        test::TestHDRBloom* bloomHDRTest = new test::TestHDRBloom(window);
        test::TestDeferredRendering* deferredRenderingTest = new test::TestDeferredRendering(window);
        test::TestSSAO* ssaoTest = new test::TestSSAO(window);
        //test::TestTemplate* templateTest = new test::TestTemplate(window);
        // Register all test sandboxes
        testMenu->RegisterTest<test::TestClearColour*>("Change background colour", (test::TestClearColour*) clearColourTest);
        testMenu->RegisterTest<test::TestTexture2D*>("Textured cube test", (test::TestTexture2D*) texture2DTest);
        testMenu->RegisterTest<test::TestFPSCamera*>("First person camera (map)", (test::TestFPSCamera*) cameraTest);
        testMenu->RegisterTest<test::TestPhongLighting*>("Blinn-Phong lighting model sandbox", (test::TestPhongLighting*) phongLightingTest);
        testMenu->RegisterTest<test::TestModelLoading*>("Model loading test", (test::TestModelLoading*) modelLoadingTest);
        testMenu->RegisterTest<test::TestManualFramebuffer*>("Rear View Framebuffer test", (test::TestManualFramebuffer*) framebufferTest);
        testMenu->RegisterTest<test::TestCubemapping*>("Skybox/Cubemapping test", (test::TestCubemapping*) cubemappingTest);
        testMenu->RegisterTest<test::TestGeometryShader*>("Manual Geometry Shader", (test::TestGeometryShader*) geometryShaderTest);
        testMenu->RegisterTest<test::TestInstancedRendering*>("Instanced Rendering test", (test::TestInstancedRendering*) instancedRenderingTest);
        testMenu->RegisterTest<test::TestShadowMapping*>("Orthographic Shadow Mapping", (test::TestShadowMapping*) shadowMappingTest);
        testMenu->RegisterTest<test::TestPointShadowMapping*>("Perspective Shadow Mapping", (test::TestPointShadowMapping*) pointShadowMappingTest);
        testMenu->RegisterTest<test::TestParallaxNormalMapping*>("Parallax and Normal Mapping", (test::TestParallaxNormalMapping*) parallaxNormalTest);
        testMenu->RegisterTest<test::TestHDRBloom*>("HDR and Bloom", (test::TestHDRBloom*) bloomHDRTest);
        testMenu->RegisterTest<test::TestDeferredRendering*>("Deferred Rendering", (test::TestDeferredRendering*) deferredRenderingTest);
        testMenu->RegisterTest<test::TestSSAO*>("Ambient Occlusion (SSAO)", (test::TestSSAO*) ssaoTest);
        //testMenu->RegisterTest<test::TestTemplate*>("Test Template", (test::TestTemplate*) templateTest);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window)) 
        {
            /* User key input processing */
            processInput(window);

            // Start each new frame by clearing
            float* clearColour = clearColourTest->GetClearColour();
            GLCall(glClearColor(clearColour[0], clearColour[1], clearColour[2], clearColour[3]));
            renderer.Clear();

            ImGui_ImplGlfwGL3_NewFrame();
            if (activeTest)
            {
                activeTest->OnUpdate(0.0f);
                //double timestampBefore = glfwGetTime();
                //std::cout << "LOG: Before OnRender" << std::endl;
                activeTest->OnRender();
                //double onRenderTime = (glfwGetTime() - timestampBefore);
                //std::cout << "LOG: After OnRender, took " << onRenderTime << " ms" <<std::endl;
                //std::cout << std::endl << std::endl << std::endl;
                ImGui::Begin("Test directory");
                if (activeTest != testMenu && ImGui::Button("<-"))
                {
                    activeTest = testMenu;
                    activeTest->OnActivated();
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
        // TODO: deallocate heap memory (tests)
        // Release OpenGL resources on termination
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

    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
    {
        activeTest = testMenu;
        activeTest->OnActivated();
    }
        
    // Toggle between wireframe/fully shaded modes
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    test::TestMenu* testMenu = test::TestMenu::GetInstance();
    testMenu->SetScreenDimensions(width, height);
    glViewport(0, 0, width, height);
}