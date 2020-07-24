#include "TestGeometryShader.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"
#include <vendor\stb_image\stb_image.h>

namespace test
{
	// Function declarations
	void mouse_callbackGeometryShader(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackGeometryShader(GLFWwindow* window, double xOffset, double yOffset);
	void processInputGeometryShader(GLFWwindow* window);
	void mouse_button_callbackGeometryShader(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestGeometryShader* TestGeometryShader::instance;

	TestGeometryShader::TestGeometryShader(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		modelLoaded(false),
		m_BackpackModel(nullptr),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)),
		m_Camera(Camera(m_CameraPos, 100.0f)),
		m_Shader(new Shader("res/shaders/HelloGeometry.shader"))
	{
		instance = this;
	}

	TestGeometryShader::~TestGeometryShader()
	{
	}

	void TestGeometryShader::OnUpdate(float deltaTime)
	{
	}

	void TestGeometryShader::OnRender()
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
		processInputGeometryShader(m_MainWindow);

		Renderer renderer;

		// Bind shader and set any 'per frame' uniforms
		m_Shader->Bind();
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 400.0f);
		m_Shader->SetMatrix4f("model", modelMatrix);
		m_Shader->SetMatrix4f("view", viewMatrix);
		m_Shader->SetMatrix4f("proj", projMatrix);
		m_Shader->SetFloat("u_time", glfwGetTime());

		// Load model's uniforms and render the loaded backpack model
		m_BackpackModel->Draw(m_Shader);
	}

	void TestGeometryShader::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestGeometryShader::OnActivated()
	{
		// Only want to load the model the first time we activate this OpenGL test
		if (!modelLoaded)
		{
			// Flip texture along y axis before loading
			stbi_set_flip_vertically_on_load(true);
			m_BackpackModel = new Model((char*)"res/models/backpack/backpack.obj");
			modelLoaded = true;
		}

		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Enable gl_PointSize in vertex shader
		//glEnable(GL_PROGRAM_POINT_SIZE);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);
		// Enable blending
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		
		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackGeometryShader);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackGeometryShader);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackGeometryShader);
	}

	void scroll_callbackGeometryShader(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestGeometryShader* geometryShaderTest = test::TestGeometryShader::GetInstance();
		Camera* geometryShaderCamera = geometryShaderTest->GetCamera();
		geometryShaderCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackGeometryShader(GLFWwindow* window, double xpos, double ypos)
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

		test::TestGeometryShader* geometryShaderTest = test::TestGeometryShader::GetInstance();
		Camera* geometryShaderCamera = geometryShaderTest->GetCamera();
		geometryShaderCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackGeometryShader(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestGeometryShader* geometryShaderTest = test::TestGeometryShader::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputGeometryShader(GLFWwindow* window) {
		test::TestGeometryShader* geometryShaderTest = test::TestGeometryShader::GetInstance();
		Camera* geometryShaderCamera = geometryShaderTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			geometryShaderCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			geometryShaderCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			geometryShaderCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			geometryShaderCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
}

