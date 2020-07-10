#include "TestShadowMapping.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackShadowMapping(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackShadowMapping(GLFWwindow* window, double xOffset, double yOffset);
	void processInputShadowMapping(GLFWwindow* window);
	void mouse_button_callbackShadowMapping(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestShadowMapping* TestShadowMapping::instance;

	TestShadowMapping::TestShadowMapping(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_Shader(new Shader("res/shaders/BasicShadowMapping.shader")),
		m_CubeTexture(new Texture("res/textures/metal_border_container_texture.png", false)), // TODO
		m_GroundTexture(new Texture("res/textures/wooden_floor_texture.png", false)),
		m_VA_Cube(new VertexArray()),
		m_VA_Ground(new VertexArray()),
		m_IsFlashlightOn(true),
		m_FlashlightColour(glm::vec3(1.0f)), m_FlashlightDiffuseIntensity(glm::vec3(1.0f)),
		m_FlashlightAmbientIntensity(glm::vec3(0.4f)), m_FlashlightSpecularIntensity(glm::vec3(0.2f)),
		m_FlashlightDiffuseColour(m_FlashlightColour* m_FlashlightDiffuseIntensity),
		m_FlashlightAmbientColour(m_FlashlightDiffuseColour* m_FlashlightAmbientIntensity)
	{
		instance = this;

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

		float cubeVertices[] = {
			// positions      --  tex coords   --    normals
			   0.5,  0.5, -0.5,    1.0, 1.0,      0.0, 0.0, -1.0, // Cube back
			  -0.5, -0.5, -0.5,    0.0, 0.0,      0.0, 0.0, -1.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,      0.0, 0.0, -1.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,      0.0, 0.0, -1.0,
											     			   
			  -0.5, -0.5,  0.5,    0.0, 0.0,      0.0, 0.0, 1.0, // Cube front
			   0.5,  0.5,  0.5,    1.0, 1.0,      0.0, 0.0, 1.0,
			  -0.5,  0.5,  0.5,    0.0, 1.0,      0.0, 0.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,      0.0, 0.0, 1.0,
											     			   
			  -0.5, -0.5, -0.5,    0.0, 0.0,      -1.0, 0.0, 0.0, // Cube left
			  -0.5,  0.5,  0.5,    1.0, 1.0,      -1.0, 0.0, 0.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,      -1.0, 0.0, 0.0,
			  -0.5, -0.5,  0.5,    1.0, 0.0,      -1.0, 0.0, 0.0,
											     			   
			   0.5, -0.5,  0.5,    0.0, 0.0,      1.0, 0.0, 0.0, // Cube right
			   0.5,  0.5, -0.5,    1.0, 1.0,      1.0, 0.0, 0.0,
			   0.5,  0.5,  0.5,    0.0, 1.0,      1.0, 0.0, 0.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,      1.0, 0.0, 0.0,
											     			   
			   0.5,  0.5,  0.5,    0.0, 0.0,      0.0, 1.0, 0.0, // Cube top
			  -0.5,  0.5, -0.5,    1.0, 1.0,      0.0, 1.0, 0.0,
			  -0.5,  0.5,  0.5,    1.0, 0.0,      0.0, 1.0, 0.0,
			   0.5,  0.5, -0.5,    0.0, 1.0,      0.0, 1.0, 0.0,
											     			   
			  -0.5, -0.5,  0.5,    0.0, 0.0,      0.0, -1.0, 0.0, // Cube bottom
			   0.5, -0.5, -0.5,    1.0, 1.0,      0.0, -1.0, 0.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,      0.0, -1.0, 0.0,
			  -0.5, -0.5, -0.5,    0.0, 1.0,      0.0, -1.0, 0.0,
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

		// Ground Vertex Array setup
		m_VA_Ground->Bind();
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Ground = new VertexBuffer(groundVertices, 8 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout groundVBLayout;
		groundVBLayout.Push<float>(3); // Vertex position,		vec3
		groundVBLayout.Push<float>(2); // Texture coordinates,	vec2
		groundVBLayout.Push<float>(3); // Normals,				vec3
		m_VA_Ground->AddBuffer(*m_VB_Ground, groundVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Ground = new IndexBuffer(groundIndices, 6);

		// Cube Vertex Array setup
		m_VA_Cube->Bind();
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Cube = new VertexBuffer(cubeVertices, 8 * 4 * 6 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout cubeVBLayout;
		cubeVBLayout.Push<float>(3); // Vertex position,	 vec3
		cubeVBLayout.Push<float>(2); // Texture coordinates, vec2
		cubeVBLayout.Push<float>(3); // Normals,			 vec3
		m_VA_Cube->AddBuffer(*m_VB_Cube, cubeVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Cube = new IndexBuffer(cubeIndices, 6 * 6);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestShadowMapping::~TestShadowMapping()
	{
	}

	void TestShadowMapping::OnUpdate(float deltaTime)
	{
	}

	void TestShadowMapping::OnRender()
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
		processInputShadowMapping(m_MainWindow);

		Renderer renderer;

		// Set per-frame uniforms
		m_Shader->Bind();

		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 800.0f);
		m_Shader->SetMatrix4f("model", modelMatrix);
		m_Shader->SetMatrix4f("view", viewMatrix);
		m_Shader->SetMatrix4f("proj", projMatrix);
		
		// Update camera's viewing position each frame
		m_Shader->SetVec3f("viewPos", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);

		// Flashlight's properties
		//
		m_Shader->SetBool("u_Flashlight.on", m_IsFlashlightOn);
		m_Shader->SetVec3("u_Flashlight.ambient", m_FlashlightAmbientColour);
		m_Shader->SetVec3("u_Flashlight.diffuse", m_FlashlightDiffuseColour);
		m_Shader->SetVec3("u_Flashlight.specular", m_FlashlightSpecularIntensity);
		// Flashlight attenuation properties
		m_Shader->SetFloat("u_Flashlight.constant", 1.0f);
		m_Shader->SetFloat("u_Flashlight.linear", 0.02f);
		m_Shader->SetFloat("u_Flashlight.quadratic", 0.01f);
		// Flashlight position and direction
		glm::vec3 flashlightPosition = m_Camera.Position;
		m_Shader->SetVec3f("u_Flashlight.position", flashlightPosition.x, flashlightPosition.y, flashlightPosition.z);
		glm::vec3 flashlightDirection = m_Camera.Front;
		m_Shader->SetVec3f("u_Flashlight.direction", flashlightDirection.x, flashlightDirection.y, flashlightDirection.z);
		// Flashlight cutoff angle
		m_Shader->SetFloat("u_Flashlight.cutOff", glm::cos(glm::radians(1.0f)));
		m_Shader->SetFloat("u_Flashlight.outerCutOff", glm::cos(glm::radians(35.0f)));

		// Render the cube
		m_CubeTexture->Bind(1);
		m_Shader->SetInt("u_Material.diffuse", 1);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_Shader);

		// Render the ground
		m_GroundTexture->BindAndSetRepeating(0);
		m_Shader->SetInt("u_Material.diffuse", 0);
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_Shader);
	}

	void TestShadowMapping::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("// TODO UI");
	}

	void TestShadowMapping::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//  Reset all uniforms
		m_Shader->Bind();
		m_Shader->SetVec3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
		m_Shader->SetFloat("u_Material.shininess", 64.0f);

		// Directional light
		m_Shader->SetVec3f("u_DirLight.direction", -1.0f, 1.0f, 1.0);
		m_Shader->SetVec3f("u_DirLight.ambient", 0.05f, 0.05f, 0.05f);
		m_Shader->SetVec3f("u_DirLight.diffuse", 0.3f, 0.3f, 0.3f);
		m_Shader->SetVec3f("u_DirLight.specular", 0.0f, 0.0f, 0.0f);

		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackShadowMapping);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackShadowMapping);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackShadowMapping);
	}

	void scroll_callbackShadowMapping(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestShadowMapping* shadowMapping = test::TestShadowMapping::GetInstance();
		Camera* shadowMappingCamera = shadowMapping->GetCamera();
		shadowMappingCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackShadowMapping(GLFWwindow* window, double xpos, double ypos)
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

		test::TestShadowMapping* shadowMapping = test::TestShadowMapping::GetInstance();
		Camera* shadowMappingCamera = shadowMapping->GetCamera();
		shadowMappingCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackShadowMapping(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestShadowMapping* shadowMapping = test::TestShadowMapping::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputShadowMapping(GLFWwindow* window) {
		test::TestShadowMapping* shadowMapping = test::TestShadowMapping::GetInstance();
		Camera* shadowMappingCamera = shadowMapping->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(RIGHT, deltaTime);
	}
}

