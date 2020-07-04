#include "TestCubemapping.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"
#include <vendor\stb_image\stb_image.h>

namespace test
{
	// Function declarations
	void mouse_callbackCubemapping(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackCubemapping(GLFWwindow* window, double xOffset, double yOffset);
	void processInputCubemapping(GLFWwindow* window);
	void mouse_button_callbackCubemapping(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestCubemapping* TestCubemapping::instance;

	TestCubemapping::TestCubemapping(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(6.0f, 0.0f, -10.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f, glm::vec3(0.0f, 1.0f, 0.0f), 90.0f)),
		m_CubeShader(new Shader("res/shaders/EnvMapping.shader")),
		m_SkyboxShader(new Shader("res/shaders/Skybox.shader")),
		m_CubeTexture(new Texture("res/textures/metal_scratched_texture.png")),
		m_VA_Cube(new VertexArray()),
		m_VA_Skybox(new VertexArray())
	{
		instance = this;

		float skyboxVertices[] = {
			//   Positions        
			   1.0f,  1.0f, -1.0f, 
			  -1.0f, -1.0f, -1.0f, 
			  -1.0f,  1.0f, -1.0f, 
			   1.0f, -1.0f, -1.0f, 

			  -1.0f, -1.0f,  1.0f, 
			   1.0f,  1.0f,  1.0f, 
			  -1.0f,  1.0f,  1.0f, 
			   1.0f, -1.0f,  1.0f, 

			  -1.0f, -1.0f, -1.0f, 
			  -1.0f,  1.0f,  1.0f, 
			  -1.0f,  1.0f, -1.0f, 
			  -1.0f, -1.0f,  1.0f, 

			   1.0f, -1.0f,  1.0f, 
			   1.0f,  1.0f, -1.0f, 
			   1.0f,  1.0f,  1.0f, 
			   1.0f, -1.0f, -1.0f, 

			   1.0f,  1.0f,  1.0f, 
			  -1.0f,  1.0f, -1.0f, 
			  -1.0f,  1.0f,  1.0f, 
			   1.0f,  1.0f, -1.0f, 

			  -1.0f, -1.0f,  1.0f, 
			   1.0f, -1.0f, -1.0f, 
			   1.0f, -1.0f,  1.0f, 
			  -1.0f, -1.0f, -1.0f, 
		};

		unsigned int skyboxIndices[]{
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

		float cubeVertices[] = {
			// positions     --   tex coords  --   normals
			   0.5,  0.5, -0.5,    1.0, 1.0,	0.0, 0.0, -1.0, // Cube back
			  -0.5, -0.5, -0.5,    0.0, 0.0,	0.0, 0.0, -1.0, 
			  -0.5,  0.5, -0.5,    0.0, 1.0,	0.0, 0.0, -1.0, 
			   0.5, -0.5, -0.5,    1.0, 0.0,	0.0, 0.0, -1.0, 
												 
			  -0.5, -0.5,  0.5,    0.0, 0.0,	0.0, 0.0, 1.0, // Cube front
			   0.5,  0.5,  0.5,    1.0, 1.0,	0.0, 0.0, 1.0,
			  -0.5,  0.5,  0.5,    0.0, 1.0,	0.0, 0.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,	0.0, 0.0, 1.0,
												
			  -0.5, -0.5, -0.5,    0.0, 0.0,	-1.0, 0.0, 0.0, // Cube left
			  -0.5,  0.5,  0.5,    1.0, 1.0,	-1.0, 0.0, 0.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,	-1.0, 0.0, 0.0,
			  -0.5, -0.5,  0.5,    1.0, 0.0,	-1.0, 0.0, 0.0,
												 
			   0.5, -0.5,  0.5,    0.0, 0.0,	1.0, 0.0, 0.0, // Cube right
			   0.5,  0.5, -0.5,    1.0, 1.0,	1.0, 0.0, 0.0,
			   0.5,  0.5,  0.5,    0.0, 1.0,	1.0, 0.0, 0.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,	1.0, 0.0, 0.0,
												 
			   0.5,  0.5,  0.5,    0.0, 0.0,	0.0, 1.0, 0.0, // Cube top
			  -0.5,  0.5, -0.5,    1.0, 1.0,	0.0, 1.0, 0.0,
			  -0.5,  0.5,  0.5,    1.0, 0.0,	0.0, 1.0, 0.0,
			   0.5,  0.5, -0.5,    0.0, 1.0,	0.0, 1.0, 0.0,
												
			  -0.5, -0.5,  0.5,    0.0, 0.0,	0.0, -1.0, 0.0, // Cube bottom
			   0.5, -0.5, -0.5,    1.0, 1.0,	0.0, -1.0, 0.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,	0.0, -1.0, 0.0,
			  -0.5, -0.5, -0.5,    0.0, 1.0,	0.0, -1.0, 0.0,
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
		m_VB_Cube = new VertexBuffer(cubeVertices, 8 * 4 * 6 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout cubeVBLayout;
		cubeVBLayout.Push<float>(3); // Vertex positions,	 vec3
		cubeVBLayout.Push<float>(2); // Texture coordinates, vec2
		cubeVBLayout.Push<float>(3); // Vertex normals,		 vec3
		m_VA_Cube->AddBuffer(*m_VB_Cube, cubeVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Cube = new IndexBuffer(cubeIndices, 6 * 6);

		// Skybox Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Skybox = new VertexBuffer(skyboxVertices, 3 * 4 * 6 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout skyboxVBLayout;
		skyboxVBLayout.Push<float>(3); // Vertex positions,  vec3
		m_VA_Skybox->AddBuffer(*m_VB_Skybox, skyboxVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Skybox = new IndexBuffer(skyboxIndices, 6 * 6);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestCubemapping::~TestCubemapping()
	{
	}

	void TestCubemapping::OnUpdate(float deltaTime)
	{
	}

	void TestCubemapping::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 1.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process keyboard inputs
		processInputCubemapping(m_MainWindow);

		Renderer renderer;

		// First, render the scene normally
		//
		// Set per-frame uniforms
		m_CubeShader->Bind();
		// Model, View, Projection matrices
		glm::mat4 modelMatrix = glm::mat4(2.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_CubeShader->SetMatrix4f("modelMatrix", modelMatrix);
		m_CubeShader->SetMatrix4f("viewMatrix",  viewMatrix);
		m_CubeShader->SetMatrix4f("projMatrix",  projMatrix);
		m_CubeShader->SetVec3("u_CameraPos",  m_Camera.Position);
		renderer.Draw(*m_VA_Cube, *m_IB_Cube, *m_CubeShader);

		// Then render the skybox with depth testing at LEQUAL (and set z component to be (w / w) = 1.0 = max depth in vertex shader)
		glDepthFunc(GL_LEQUAL);
		m_SkyboxShader->Bind();
		// Model, View, Projection matrices
		modelMatrix = glm::mat4(1.0f);
		viewMatrix = glm::mat4(glm::mat3(m_Camera.GetViewMatrix())); // Gets rid of any translation
		projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_SkyboxShader->SetMatrix4f("modelMatrix", modelMatrix);
		m_SkyboxShader->SetMatrix4f("viewMatrix",  viewMatrix);
		m_SkyboxShader->SetMatrix4f("projMatrix",  projMatrix);
		renderer.Draw(*m_VA_Skybox, *m_IB_Skybox, *m_SkyboxShader);
		glDepthFunc(GL_LESS);

	}

	void TestCubemapping::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestCubemapping::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//  Reset all uniforms
		//
		// Cube environment mapping shader
		m_CubeShader->Bind();
		m_CubeShader->SetInt("u_SkyboxTexture", 3);
		m_CubeShader->SetInt("u_SurfaceTexture", 1);
		// Skybox shader
		m_SkyboxShader->Bind();
		m_SkyboxShader->SetInt("u_SkyboxTexture", 3);

		// Textures
		// TODO check for memory leak here
		m_SkyboxTexture = new Texture(std::vector<std::string>({ "res/textures/example_skybox/right.jpg",
																"res/textures/example_skybox/left.jpg",
																"res/textures/example_skybox/top.jpg",
																"res/textures/example_skybox/bottom.jpg",
																"res/textures/example_skybox/front.jpg",
																"res/textures/example_skybox/back.jpg" }));
		m_CubeTexture->Bind(1);
		m_SkyboxTexture->BindCubemap(3);
		
		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackCubemapping);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackCubemapping);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackCubemapping);
	}

	void scroll_callbackCubemapping(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestCubemapping* cubemappingTest = test::TestCubemapping::GetInstance();
		Camera* cubemappingCamera = cubemappingTest->GetCamera();
		cubemappingCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackCubemapping(GLFWwindow* window, double xpos, double ypos)
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

		test::TestCubemapping* cubemappingTest = test::TestCubemapping::GetInstance();
		Camera* cubemappingCamera = cubemappingTest->GetCamera();
		cubemappingCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackCubemapping(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestCubemapping* cubemappingTest = test::TestCubemapping::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputCubemapping(GLFWwindow* window) {
		test::TestCubemapping* cubemappingTest = test::TestCubemapping::GetInstance();
		Camera* cubemappingCamera = cubemappingTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cubemappingCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cubemappingCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cubemappingCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cubemappingCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
}

