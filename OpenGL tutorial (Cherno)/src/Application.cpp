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

// Function declarations
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void processInput(GLFWwindow* window);

// Global vars
static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

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
    window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
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
        float vertices[] = {
           // positions -- tex coords 
             -1.0, -1.0,    0.0, 0.0,
              1.0,  1.0,    1.0, 1.0,
             -1.0,  1.0,    0.0, 1.0,
              1.0, -1.0,    1.0, 0.0,

          /*-0.5, -0.5,    0.0, 0.0,
             0.5,  0.5,    1.0, 1.0,
            -0.5,  0.5,    0.0, 1.0,
             0.5, -0.5,    1.0, 0.0,*/
        };

        /*float texCoords[] = {
            0.0, 0.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 1.0,
        };*/

        unsigned int indices[]{
            0, 1, 2,
            3, 0, 1
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        // Create Vertex Array
        VertexArray VA;
        // Create Vertex Buffer
        VertexBuffer VB(vertices, 4 * 4 * sizeof(float));

        // Create and associate the layout (Vertex Attribute Pointer)
        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);
        VA.AddBuffer(VB, layout);

        // Create index buffer
        IndexBuffer IB(indices, 6);

        // Shader programs
        Shader shader("res/shaders/Basic.shader");
        shader.Bind();

        // Load texture and set uniform in shader
        Texture texture("res/textures/tree_render_texture.png");
        // Texture texture("res/textures/metal_border_container_texture.png");
        texture.Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
        shader.SetUniform1i("u_Texture", 0);

        // Unbind everything
        VA.Unbind();
        VB.Unbind();
        IB.Unbind();
        shader.Unbind();

        Renderer renderer;

        // ImGui initialization stuff
        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init(window, true);
        ImGui::StyleColorsDark();
        // Varibles to tweak with ImGui
        glm::vec3 modelTranslation(0.0f, 0.0f, 0.0f);
        float modelRotationZ = 0.0f;
        float modelScale = 1.0f;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* User key input processing */
            processInput(window);

            // Start each new frame by clearing
            renderer.Clear();

            ImGui_ImplGlfwGL3_NewFrame();

            // Bind shader and set its uniforms
			shader.Bind();
			shader.SetUniform4f("u_Color", 0.2f, ((sin(glfwGetTime()) + 1.0f) / 2.0f), 0.8f, 1.0f);
            //
			// Create model, view, projection matrices 
		    // Send combined MVP matrix to shader
			glm::mat4 model = glm::mat4(1.0);
            model = glm::translate(glm::mat4(1.0), modelTranslation);
            model = glm::rotate(model, glm::radians(modelRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(modelScale));
            glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
            glm::mat4 proj = glm::ortho(0.0f, (float)SCREEN_WIDTH / 100, 0.0f, (float)SCREEN_HEIGHT / 100, -1.0f, 1.0f);
			glm::mat4 MVP_matrix = proj * view * model;
			shader.SetMatrix4f("u_MVP", MVP_matrix);

			// Renderer draw call
			renderer.Draw(VA, IB, shader);

            // ImGui window rendering
            {
                ImGui::SliderFloat3("Model translation", &modelTranslation.x, 0.0f, 10.0f);
                ImGui::SliderFloat("Model Z axiz rotation", &modelRotationZ, -90.0f, 90.0f);
                ImGui::SliderFloat("Model scale", &modelScale, -1.0f, 10.0f);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);



                // EXAMPLE WINDOW BELOW
                //static float f = 0.0f;
                //static int counter = 0;
                //ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
                //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
                //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
                //ImGui::Checkbox("Another Window", &show_another_window);

                //if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
                //    counter++;
                //ImGui::SameLine();
                //ImGui::Text("counter = %d", counter);
                //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }

            ImGui::Render();
            ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

        // Release resources on termination
        //
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