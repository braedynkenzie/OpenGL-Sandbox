#include "TestPhongLighting.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <vendor\stb_image\stb_image.h>
#include <tests\TestClearColour.h>

#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackPhongTest(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackPhongTest(GLFWwindow* window, double xOffset, double yOffset);
	void processInputPhongTest(GLFWwindow* window);

	// Init static variable
	TestPhongLighting* TestPhongLighting::instance;

	TestPhongLighting::TestPhongLighting(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow), 
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)), 
		m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), 
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)), 
		m_Camera(Camera(m_CameraPos, 110.0f))
	{
		instance = this;

		// Mouse cursor should start in the middle of the window
		lastCursorX = SCREEN_WIDTH / 2.0f;
		lastCursorY = SCREEN_HEIGHT / 2.0f;

		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackPhongTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackPhongTest);

		// Create vertice positions
		float vertices[] = {
		 //       positions     --   tex coords 
			  -80.0, -10.0, -80.0,    0.0, 1.0, // Floor
			   80.0, -10.0,  80.0,    1.0, 0.0,
			  -80.0, -10.0,  80.0,    0.0, 0.0,
			   80.0, -10.0, -80.0,    1.0, 1.0,
		};

		unsigned int indices[]{
			0, 1, 2,
			3, 0, 1,
		};

		m_VA = std::make_unique<VertexArray>();

		// Init Vertex Buffer and bind to Vertex Array (m_VA)
		m_VB = std::make_unique<VertexBuffer>(vertices, 5 * 4 * sizeof(float));

		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout layout;
		layout.Push<float>(3); // Vertex position vec3
		layout.Push<float>(2); // Texture coordinates vec2
		m_VA->AddBuffer(*m_VB, layout);

		// Init index buffer and bind to Vertex Array (m_VA)
		m_IB = std::make_unique<IndexBuffer>(indices, 6);

		// Load shader
		m_Shader = std::make_unique<Shader>("res/shaders/BasicPhongModel.shader");

		// Flip texture along y axis before loading
		stbi_set_flip_vertically_on_load(true);

		// Bind shader program and set uniforms
		m_Shader->Bind();
		m_Texture = std::make_unique<Texture>("res/textures/high_res_world_map_texture.png");
		m_Texture->Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
		m_Shader->SetUniform1i("u_Texture0", 0);

		// Unbind everything
		m_VA->Unbind();
		m_VB->Unbind();
		m_IB->Unbind();
		m_Shader->Unbind();

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestPhongLighting::~TestPhongLighting()
	{
	}

	void TestPhongLighting::OnUpdate(float deltaTime)
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void TestPhongLighting::OnRender()
	{
		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process WASD keyboard camera movement
		processInputPhongTest(m_MainWindow);

		float* clearColour = test::TestClearColour::GetClearColour();
		GLCall(glClearColor(clearColour[0], clearColour[1], clearColour[2], clearColour[3]));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		Renderer renderer;

		// Bind shader and set its per frame uniforms
		m_Shader->Bind();
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(50.0, 0.0, 36.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 proj = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		glm::mat4 view = m_Camera.GetViewMatrix();
		glm::mat4 MVP_matrix = proj * view * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);
		renderer.Draw(*m_VA, *m_IB, *m_Shader);
	}

	void TestPhongLighting::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestPhongLighting::OnActivated()
	{
		// Bind shader program and reset any uniforms
		m_Shader->Bind();
		m_Texture = std::make_unique<Texture>("res/textures/high_res_world_map_texture.png");
		m_Texture->Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
		m_Shader->SetUniform1i("u_Texture0", 0);

		// Reset all callbacks
		// Callback function for keyboard inputs
		processInputPhongTest(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackPhongTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackPhongTest);
	}

	void scroll_callbackPhongTest(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestPhongLighting* lightingTest = test::TestPhongLighting::GetInstance();
		Camera* phongCamera = lightingTest->GetCamera();
		phongCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackPhongTest(GLFWwindow* window, double xpos, double ypos)
	{
		// Fixes first mouse cursor capture by OpenGL window
		if (firstMouseCapture)
		{
			lastCursorX = xpos;
			lastCursorY = ypos;
			firstMouseCapture = false;
		}
		float xOffset = xpos - lastCursorX;
		float yOffset = lastCursorY - ypos; // reverse the y-coordinates
		float cursorSensitivity = 0.08f;
		xOffset *= cursorSensitivity;
		yOffset *= cursorSensitivity;
		yaw += xOffset;
		pitch += yOffset;
		lastCursorX = xpos;
		lastCursorY = ypos;

		test::TestPhongLighting* lightingTest = test::TestPhongLighting::GetInstance();
		Camera* phongCamera = lightingTest->GetCamera();
		phongCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void processInputPhongTest(GLFWwindow* window) {
		test::TestPhongLighting* lightingTest = test::TestPhongLighting::GetInstance();
		Camera* phongCamera = lightingTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			phongCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			phongCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			phongCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			phongCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
}