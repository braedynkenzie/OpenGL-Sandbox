#include "TestModelLoading.h"
#include "Renderer.h"

#include <tests\TestModelLoading.h>
#include <tests\TestClearColour.h>

#include "Globals.h"


namespace test
{
	// Function declarations
	void mouse_callback_ModelLoadingTest(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback_ModelLoadingTest(GLFWwindow* window, double xOffset, double yOffset);
	void processInput_ModelLoadingTest(GLFWwindow* window);

	// Init static variable
	TestModelLoading* TestModelLoading::instance;

	TestModelLoading::TestModelLoading(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_BackpackPos(glm::vec3(0.0f, 0.0f, -2.0f)), 
		// Load model -- TODO destruct
		m_BackpackModel(new Model((char*)"res/models/backpack/backpack.obj")), 
		// Load shader -- TODO destruct
		m_BackpackShader(new Shader("res/shaders/Backpack.shader")),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)), 
		m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
		m_Camera(new Camera(m_CameraPos, 75.0f)), // TODO destruct
		m_IsFlashlightOn(true),
		m_FlashlightColour(glm::vec3(1.0f)), m_fl_diffuseIntensity(glm::vec3(1.0f)),
		m_fl_ambientIntensity(glm::vec3(0.4f)), m_fl_specularIntensity(glm::vec3(0.2f)),
		m_fl_diffuseColour(m_FlashlightColour* m_fl_diffuseIntensity),
		m_fl_ambientColour(m_fl_diffuseColour* m_fl_ambientIntensity)
	{
		instance = this;

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestModelLoading::~TestModelLoading()
	{
	}

	void TestModelLoading::OnUpdate(float deltaTime)
	{
	}

	void TestModelLoading::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 1.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Setup and render the loaded backpack model
		// Use the backpack model shader
		m_BackpackShader->Bind();

		// Set uniforms
		// View/Projection transformations
		glm::mat4 proj = glm::perspective(glm::radians(m_Camera->Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = m_Camera->GetViewMatrix();
		m_BackpackShader->SetMatrix4f("proj", proj);
		m_BackpackShader->SetMatrix4f("view", view);

		// Render the loaded model
		glm::mat4 loaded_model_matrix = glm::mat4(1.0f);
		loaded_model_matrix = glm::translate(loaded_model_matrix, glm::vec3(m_BackpackPos));
		loaded_model_matrix = glm::scale(loaded_model_matrix, glm::vec3(0.5f));
		m_BackpackShader->SetMatrix4f("model", loaded_model_matrix);
		m_BackpackShader->SetVec3("viewPos", m_Camera->Position);
		
		// Flashlight position and direction
		m_BackpackShader->SetVec3("u_Flashlight.position", m_Camera->Position);
		m_BackpackShader->SetVec3("u_Flashlight.direction", m_Camera->Front);

		// Render the loaded backpack model
		m_BackpackModel->Draw(m_BackpackShader);

	}

	void TestModelLoading::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestModelLoading::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Bind shader program and set uniforms
		m_BackpackShader->Bind();
		// Set material struct properties
		m_BackpackShader->SetVec3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
		m_BackpackShader->SetFloat("u_Material.shininess", 16.0f);
		//
		// Flashlight properties
		m_BackpackShader->SetBool("u_Flashlight.on", m_IsFlashlightOn);
		m_BackpackShader->SetVec3("u_Flashlight.ambient", m_fl_ambientColour);
		m_BackpackShader->SetVec3("u_Flashlight.diffuse", m_fl_diffuseColour);
		m_BackpackShader->SetVec3("u_Flashlight.specular", m_fl_specularIntensity);
		// Flashlight attenuation properties
		m_BackpackShader->SetFloat("u_Flashlight.constant", 1.0f);
		m_BackpackShader->SetFloat("u_Flashlight.linear", 0.09f);
		m_BackpackShader->SetFloat("u_Flashlight.quadratic", 0.032f);
		// Flashlight cutOff angle
		m_BackpackShader->SetFloat("u_Flashlight.cutOff", glm::cos(glm::radians(5.0f)));
		m_BackpackShader->SetFloat("u_Flashlight.outerCutOff", glm::cos(glm::radians(20.0f)));

		// Reset all callbacks
		// Callback function for keyboard inputs
		processInput_ModelLoadingTest(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callback_ModelLoadingTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callback_ModelLoadingTest);
	}

	void scroll_callback_ModelLoadingTest(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestModelLoading* modelLoadingTest = test::TestModelLoading::GetInstance();
		Camera* modelCamera = modelLoadingTest->GetCamera();
		modelCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callback_ModelLoadingTest(GLFWwindow* window, double xpos, double ypos)
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

		test::TestModelLoading* modelLoadingTest = test::TestModelLoading::GetInstance();
		Camera* modelCamera = modelLoadingTest->GetCamera();
		modelCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void processInput_ModelLoadingTest(GLFWwindow* window) {
		test::TestModelLoading* modelLoadingTest = test::TestModelLoading::GetInstance();
		Camera* modelCamera = modelLoadingTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			modelCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			modelCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			modelCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			modelCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
}

