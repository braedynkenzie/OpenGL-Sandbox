#include "TestTemplate.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>

#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackTemplate(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackTemplate(GLFWwindow* window, double xOffset, double yOffset);
	void processInputTemplate(GLFWwindow* window);
	void mouse_button_callbackTemplate(GLFWwindow* window, int button, int action, int mods);

	TestTemplate::TestTemplate(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)),
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_Shader(new Shader("res/shaders/template.shader"))
	{
		instance = this;

		// TODO here

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestTemplate::~TestTemplate()
	{
	}

	void TestTemplate::OnUpdate(float deltaTime)
	{
	}

	void TestTemplate::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 1.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void TestTemplate::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("// TODO UI");
	}

	void TestTemplate::OnActivated()
	{
		// TODO
		
		// Reset all callbacks
		// Callback function for keyboard inputs
		processInputTemplate(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackTemplate);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackTemplate);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackTemplate);
	}

	void scroll_callbackTemplate(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestTemplate* templateTest = test::TestTemplate::GetInstance();
		Camera* templateCamera = templateTest->GetCamera();
		templateCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackTemplate(GLFWwindow* window, double xpos, double ypos)
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

		test::TestTemplate* templateTest = test::TestTemplate::GetInstance();
		Camera* templateCamera = templateTest->GetCamera();
		templateCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackTemplate(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestTemplate* templateTest = test::TestTemplate::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputTemplate(GLFWwindow* window) {
		test::TestTemplate* templateTest = test::TestTemplate::GetInstance();
		Camera* templateCamera = templateTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			templateCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			templateCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			templateCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			templateCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
}

