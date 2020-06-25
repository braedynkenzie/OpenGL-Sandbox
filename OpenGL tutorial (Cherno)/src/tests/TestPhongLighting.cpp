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
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_IsFlashlightOn(true),
		m_FlashlightColour(glm::vec3(1.0f)), m_fl_diffuseIntensity(glm::vec3(1.0f)),
		m_fl_ambientIntensity(glm::vec3(0.4f)), m_fl_specularIntensity(glm::vec3(0.2f)),
		m_fl_diffuseColour( m_FlashlightColour * m_fl_diffuseIntensity), 
		m_fl_ambientColour(m_fl_diffuseColour * m_fl_ambientIntensity),
		m_PointLightPos(glm::vec3(2.0f, 2.0f, -40.0f)),
		m_pl_diffuseIntensity(glm::vec3(0.9f)), m_pl_ambientIntensity(glm::vec3(0.4f)), 
		m_pl_specularIntensity(glm::vec3(0.8f))
	{
		instance = this;

		// Mouse cursor should start in the middle of the window
		lastCursorX = SCREEN_WIDTH / 2.0f;
		lastCursorY = SCREEN_HEIGHT / 2.0f;

		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackPhongTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackPhongTest);

		// Create vertices and incdices
		float groundVertices[] = {
			//       positions      --     tex coords     --    normals
				-800.0, -10.0, -800.0,      0.0, 100.0,      0.0, 1.0, 0.0,
				 800.0, -10.0,  800.0,    100.0,   0.0,      0.0, 1.0, 0.0,
				-800.0, -10.0,  800.0,      0.0,   0.0,      0.0, 1.0, 0.0,
				 800.0, -10.0, -800.0,    100.0, 100.0,      0.0, 1.0, 0.0,
		};

		unsigned int groundIndices[]{
			0, 2, 1,
			3, 0, 1,
		};

		float pointLightVertices[] = {
			//       positions    --   tex coords
				-0.5, -0.5, -0.5,       0.0, 0.0,		// bottom front left,	0
				 0.5,  0.5, -0.5,       1.0, 1.0,		// top front right,		1
				-0.5,  0.5, -0.5,       0.0, 1.0,		// top front left,		2
				 0.5, -0.5, -0.5,       1.0, 0.0,		// bottom front right,	3
				 0.5,  0.5,  0.5,       1.0, 1.0,		// top back right,		4
				-0.5, -0.5,  0.5,       0.0, 0.0,		// bottom back left,	5
				-0.5,  0.5,  0.5,       0.0, 1.0,		// top back left,		6
				 0.5, -0.5,  0.5,       1.0, 0.0,		// bottom back right,	7
		};

		unsigned int pointLightIndices[]{
			 0, 1, 2, // Front left
			 1, 0, 3, // Front right

			 4, 5, 6, // Back left
			 5, 4, 7, // Back right

			 5, 2, 6, // Left left
			 2, 5, 0, // Left right

			 3, 4, 1, // Right left
			 4, 3, 7, // Right right

			 2, 4, 6, // Top left
			 4, 2, 1, // Top right

			 5, 3, 0, // Bottom left
			 3, 5, 7, // Bottom right
		};

		// Ground Vertex Array setup
		m_VA_Ground = std::make_unique<VertexArray>();
		// Init Vertex Buffer and bind to Vertex Array (m_VA)
		m_VB_Ground = std::make_unique<VertexBuffer>(groundVertices, 8 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout groundVBLayout;
		groundVBLayout.Push<float>(3); // Vertex position,vec3
		groundVBLayout.Push<float>(2); // Texture coordinates, vec2
		groundVBLayout.Push<float>(3); // Normals, vec3
		m_VA_Ground->AddBuffer(*m_VB_Ground, groundVBLayout);
		// Init index buffer and bind to Vertex Array (m_VA)
		m_IB_Ground = std::make_unique<IndexBuffer>(groundIndices, 6);

		// Pointlight Vertex Array setup
		m_VA_PointLight = std::make_unique<VertexArray>();
		// Init Vertex Buffer and bind to Vertex Array (m_VA)
		m_VB_PointLight = std::make_unique<VertexBuffer>(pointLightVertices, 5 * 8 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout pointLightVBLayout;
		pointLightVBLayout.Push<float>(3); // Vertex position,vec3
		pointLightVBLayout.Push<float>(2); // Texture coordinates, vec2
		m_VA_PointLight->AddBuffer(*m_VB_PointLight, pointLightVBLayout);
		// Init index buffer and bind to Vertex Array (m_VA)
		m_IB_PointLight = std::make_unique<IndexBuffer>(pointLightIndices, 6 * 6);

		// Load shaders
		m_GroundShader = std::make_unique<Shader>("res/shaders/BasicPhongModel.shader");
		m_PointLightsShader = std::make_unique<Shader>("res/shaders/PointLights.shader");

		// NOTE: Would unbind any buffers/shaders here if necessary

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
		float darknessFactor = 10.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor, 
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Point light colour
		glm::vec3 lightColor;
		lightColor.x = sin(glfwGetTime() * 1.0f) / 2.0f + 0.7f;
		lightColor.y = sin(glfwGetTime() * 0.5f) / 2.0f + 0.7f;
		lightColor.z = sin(glfwGetTime() * 0.4f) / 2.0f + 0.7f;

		Renderer renderer;

		// Bind shader and set any 'per frame' uniforms
		m_GroundShader->Bind();
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(50.0, 0.0, 36.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_GroundShader->SetMatrix4f("model", modelMatrix);
		m_GroundShader->SetMatrix4f("view", viewMatrix);
		m_GroundShader->SetMatrix4f("proj", projMatrix);

		// Update camera's viewing position each frame
		m_GroundShader->SetVec3f("viewPos", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);

		// Flashlight's properties
		//
		m_GroundShader->SetBool("u_Flashlight.on", m_IsFlashlightOn);
		m_GroundShader->SetVec3("u_Flashlight.ambient", m_fl_ambientColour);
		m_GroundShader->SetVec3("u_Flashlight.diffuse", m_fl_diffuseColour);
		m_GroundShader->SetVec3("u_Flashlight.specular", m_fl_specularIntensity);
		// Flashlight attenuation properties
		m_GroundShader->SetFloat("u_Flashlight.constant", 1.0f);
		m_GroundShader->SetFloat("u_Flashlight.linear", 0.06f);
		m_GroundShader->SetFloat("u_Flashlight.quadratic", 0.005f);
		// Flashlight position and direction
		m_GroundShader->SetVec3f("u_Flashlight.position", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);
		m_GroundShader->SetVec3f("u_Flashlight.direction", m_Camera.Front.x, m_Camera.Front.y, m_Camera.Front.z);
		// Flashlight cutoff angle
		m_GroundShader->SetFloat("u_Flashlight.cutOff", glm::cos(glm::radians(5.0f)));
		m_GroundShader->SetFloat("u_Flashlight.outerCutOff", glm::cos(glm::radians(30.0f)));
		//
		// Point light properties in m_PointLightsShader
		glm::vec3 pl_diffuseColor = lightColor * m_pl_diffuseIntensity;
		glm::vec3 pl_ambientColor = pl_diffuseColor * m_pl_ambientIntensity;
		m_PointLightsShader->Bind();
		// Model matrix: Translate and scale the light object
		glm::mat4 pointLightsModelMatrix = glm::mat4(1.0f);
		// movingLightPos = pointLightPos;
		/*if (isMovingLight) {
			movingLightPos.x *= (float)(sin(glfwGetTime()) * 3.0f);
			movingLightPos.y *= (float)(cos(glfwGetTime()) * 3.0f);
		}*/
		pointLightsModelMatrix = glm::translate(pointLightsModelMatrix, m_PointLightPos); // movingLightPos);
		pointLightsModelMatrix = glm::scale(pointLightsModelMatrix, glm::vec3(1.0f));
		m_PointLightsShader->SetMatrix4f("model", pointLightsModelMatrix);
		m_PointLightsShader->SetMatrix4f("view", viewMatrix);
		m_PointLightsShader->SetMatrix4f("proj", projMatrix);
		// Light colour uniform
		m_PointLightsShader->SetVec3("pointLightColour", lightColor * 0.8f);
		//
		// Point light properties in m_GroundShader
		m_GroundShader->Bind();
		m_GroundShader->SetVec3("pointLights[0].ambient", pl_ambientColor);
		m_GroundShader->SetVec3("pointLights[0].diffuse", pl_diffuseColor);
		m_GroundShader->SetVec3("pointLights[0].specular", m_pl_specularIntensity);
		// Point light attenuation properties
		m_GroundShader->SetFloat("pointLights[0].constant", 1.0f);
		m_GroundShader->SetFloat("pointLights[0].linear", 0.01f);
		m_GroundShader->SetFloat("pointLights[0].quadratic", 0.004f);
		// Point light position
		m_GroundShader->SetVec3("pointLights[0].position", m_PointLightPos); //movingLightPos);

		renderer.Draw(*m_VA_Ground, *m_IB_Ground, *m_GroundShader); 
		renderer.Draw(*m_VA_PointLight, *m_IB_PointLight, *m_PointLightsShader);
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
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Bind shader program and set uniforms
		m_GroundShader->Bind();
		m_Texture = std::make_unique<Texture>("res/textures/dirt_ground_texture.png");
		m_Texture->Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
		m_GroundShader->SetInt("u_Material.diffuse", 0); 
		m_GroundShader->SetVec3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
		m_GroundShader->SetFloat("u_Material.shininess", 16.0f);
		// Reset MVP matrices on activation
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(50.0, 0.0, 36.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_GroundShader->SetMatrix4f("model", modelMatrix);
		m_GroundShader->SetMatrix4f("view", viewMatrix);
		m_GroundShader->SetMatrix4f("proj", projMatrix);
		// Set texture mode to repeat
		// TODO mipmapping
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

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
			phongCamera->ProcessKeyboardForWalkingView(FORWARD, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			phongCamera->ProcessKeyboardForWalkingView(BACKWARD, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			phongCamera->ProcessKeyboardForWalkingView(LEFT, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			phongCamera->ProcessKeyboardForWalkingView(RIGHT, deltaTime, -0.2f);
	}
}
