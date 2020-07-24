#include "TestSSAmbientOcclusion.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackSSAO(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackSSAO(GLFWwindow* window, double xOffset, double yOffset);
	void processInputSSAO(GLFWwindow* window);
	void mouse_button_callbackSSAO(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestSSAO* TestSSAO::instance;

	TestSSAO::TestSSAO(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_Shader(new Shader("res/shaders/Basic.shader")),
		m_CubeTexture(new Texture("res/textures/metal_border_container_texture.png")),
		m_VA_Cube(new VertexArray())
	{
		instance = this;

		float cubeVertices[] = {
			// positions      --  tex coords 
			   0.5,  0.5, -0.5,    1.0, 1.0, // Cube back
			  -0.5, -0.5, -0.5,    0.0, 0.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,

			  -0.5, -0.5,  0.5,    0.0, 0.0, // Cube front
			   0.5,  0.5,  0.5,    1.0, 1.0,
			  -0.5,  0.5,  0.5,    0.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,

			  -0.5, -0.5, -0.5,    0.0, 0.0, // Cube left
			  -0.5,  0.5,  0.5,    1.0, 1.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,
			  -0.5, -0.5,  0.5,    1.0, 0.0,

			   0.5, -0.5,  0.5,    0.0, 0.0, // Cube right
			   0.5,  0.5, -0.5,    1.0, 1.0,
			   0.5,  0.5,  0.5,    0.0, 1.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,

			   0.5,  0.5,  0.5,    0.0, 0.0, // Cube top
			  -0.5,  0.5, -0.5,    1.0, 1.0,
			  -0.5,  0.5,  0.5,    1.0, 0.0,
			   0.5,  0.5, -0.5,    0.0, 1.0,

			  -0.5, -0.5,  0.5,    0.0, 0.0, // Cube bottom
			   0.5, -0.5, -0.5,    1.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,
			  -0.5, -0.5, -0.5,    0.0, 1.0,
		};

		unsigned int cubeIndices[]{
			0, 1, 2,
			3, 1, 0,

			4, 5, 6,
			7, 5, 4,

			8, 9, 10,
			11, 9, 8,

			12, 13, 14,
			15, 13, 12,

			16, 17, 18,
			19, 17, 16,

			20, 21, 22,
			23, 21, 20
		};

		// Cube Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Cube = new VertexBuffer(cubeVertices, 5 * 4 * 6 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout cubeVBLayout;
		cubeVBLayout.Push<float>(3); // Vertex position,	 vec3
		cubeVBLayout.Push<float>(2); // Texture coordinates, vec2
		m_VA_Cube->AddBuffer(*m_VB_Cube, cubeVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Cube = new IndexBuffer(cubeIndices, 6 * 6);
	}

	TestSSAO::~TestSSAO()
	{
	}

	void TestSSAO::OnUpdate(float deltaTime)
	{
	}

	void TestSSAO::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 2.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process keyboard inputs
		processInputSSAO(m_MainWindow);

		Renderer renderer;
		// Set per-frame uniforms
		m_Shader->Bind();
		// Model, View, Projection matrices
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		glm::mat4 MVP_matrix = projMatrix * viewMatrix * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);
		m_Shader->SetInt("u_ActiveTexture", 1);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_Shader);
	}

	void TestSSAO::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text(" - - - ");
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestSSAO::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//  Reset all uniforms
		m_Shader->Bind();
		// Textures
		m_CubeTexture->Bind(1);
		m_Shader->SetInt("u_Texture1", 1);
		m_Shader->SetInt("u_ActiveTexture", 1);
		
		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackSSAO);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackSSAO);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackSSAO);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);
		// Enable blending
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	void scroll_callbackSSAO(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();
		Camera* ssaoCamera = ssaoTest->GetCamera();
		ssaoCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackSSAO(GLFWwindow* window, double xpos, double ypos)
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

		test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();
		Camera* ssaoCamera = ssaoTest->GetCamera();
		ssaoCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackSSAO(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputSSAO(GLFWwindow* window) {
		test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();
		Camera* ssaoCamera = ssaoTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
}

