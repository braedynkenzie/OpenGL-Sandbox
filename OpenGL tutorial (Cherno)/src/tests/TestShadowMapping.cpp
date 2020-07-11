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
		m_ShadowDepthMapShader(new Shader("res/shaders/ShadowMapping.shader")),
		m_CubeTexture(new Texture("res/textures/metal_border_container_texture.png", false)), // TODO
		m_GroundTexture(new Texture("res/textures/wooden_floor_texture.png", false)),
		m_VA_Cube(new VertexArray()),
		m_VA_Ground(new VertexArray()),
		// Flashlight properties
		m_IsFlashlightOn(true),
		m_FlashlightColour(glm::vec3(1.0f)), m_FlashlightDiffuseIntensity(glm::vec3(1.0f)),
		m_FlashlightAmbientIntensity(glm::vec3(0.4f)), m_FlashlightSpecularIntensity(glm::vec3(0.2f)),
		m_FlashlightDiffuseColour(m_FlashlightColour* m_FlashlightDiffuseIntensity),
		m_FlashlightAmbientColour(m_FlashlightDiffuseColour* m_FlashlightAmbientIntensity),
		// Directional light properties
		m_DirLightDirection(glm::vec3(-1.0f, 1.0f, 1.0)),
		m_DirLightAmbient(glm::vec3(0.05f, 0.05f, 0.05f)),
		m_DirLightDiffuse(glm::vec3(0.4f, 0.4f, 0.4f)),
		m_DirLightSpecular(glm::vec3(0.1f, 0.09f, 0.1f)), // sunlight specular highlights
		// Shadow map properties
		SHADOW_MAP_WIDTH(1024),
		SHADOW_MAP_HEIGHT(1024)
	{
		instance = this;

		// Create vertices and incdices
		float groundVertices[] = {
			//       positions      --     tex coords     --    normals
				-800.0, -10.0, -800.0,      0.0, 50.0,      0.0, 1.0, 0.0,
				 800.0, -10.0,  800.0,    50.0,   0.0,      0.0, 1.0, 0.0,
				-800.0, -10.0,  800.0,      0.0,  0.0,      0.0, 1.0, 0.0,
				 800.0, -10.0, -800.0,    50.0,  50.0,      0.0, 1.0, 0.0,
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

		// First render to the shadow depth map
		//
		// Bind shadow map framebuffer
		glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		// Cull front faces while filling the shadow depth buffer to avoid Peter Panning of shadows
		glCullFace(GL_FRONT);
		// Configure matrices and shader
		m_ShadowDepthMapShader->Bind();
		float near_plane = 0.1f, far_plane = 100.0f;
		glm::mat4 lightModelMatrix = glm::mat4(1.0f);
		float movementAmount = 8.0f;
		float cubePositionX = movementAmount * sin(glfwGetTime() / 2.0f);
		float cubePositionY = movementAmount * cos(glfwGetTime() / 1.0f);
		glm::vec3 cubePosition = glm::vec3(cubePositionX, cubePositionY, 0.0f);
		lightModelMatrix = glm::translate(lightModelMatrix, cubePosition);
		lightModelMatrix = glm::scale(lightModelMatrix, glm::vec3(4.0));
		glm::mat4 lightViewMatrix = glm::lookAt(m_DirLightDirection * 10.0f, // Position of eye//m_Camera.Position,	
												glm::vec3(0.0f, 0.0f, 0.0f),   // Looking at //m_Camera.Front,		
												glm::vec3(0.0f, 1.0f, 0.0f));  // Up vector//m_Camera.Up);		    
		glm::mat4 lightProjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
		glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
		m_ShadowDepthMapShader->SetMatrix4f("lightSpaceMatrix", lightSpaceMatrix);
		m_ShadowDepthMapShader->SetMatrix4f("lightModel", lightModelMatrix);
		// Create depth map from light's POV by rendering the scene
		// Draw first cube
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_ShadowDepthMapShader);
		// Change model matrix and draw second cube
		glm::mat4 lightModelMatrix2 = glm::mat4(1.0f);
		cubePositionX *= 0.5f;
		cubePositionY = movementAmount * sin(glfwGetTime() / 2.0f);
		float cubePositionZ = movementAmount * cos(glfwGetTime() / 1.0f);
		cubePosition = glm::vec3(cubePositionX, cubePositionY, cubePositionZ);
		lightModelMatrix2 = glm::translate(lightModelMatrix2, cubePosition);
		lightModelMatrix2 = glm::scale(lightModelMatrix2, glm::vec3(4.0));
		m_ShadowDepthMapShader->SetMatrix4f("lightModel", lightModelMatrix2);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_ShadowDepthMapShader);
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_ShadowDepthMapShader);

		// Then return to the default framebuffer and render the scene as normal, using the depth map to create shadows
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Return to culling back faces
		glCullFace(GL_BACK);

		// Set per-frame uniforms
		m_Shader->Bind();
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = lightModelMatrix;
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 800.0f);
		m_Shader->SetMatrix4f("model", modelMatrix);
		m_Shader->SetMatrix4f("view", viewMatrix);
		m_Shader->SetMatrix4f("proj", projMatrix);
		// Update camera's viewing position each frame
		m_Shader->SetVec3f("viewPos", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);
		// Set lightSpaceMatrix in this shader
		m_Shader->SetMatrix4f("lightSpaceMatrix", lightSpaceMatrix);

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
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0), 2.0f * m_Camera.Right);
		flashlightPosition = glm::vec3(translationMatrix * glm::vec4(flashlightPosition, 1.0));
		m_Shader->SetVec3f("u_Flashlight.position", flashlightPosition.x, flashlightPosition.y, flashlightPosition.z);
		glm::vec3 flashlightDirection = m_Camera.Front;
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(4.0f), glm::vec3(0.0, 1.0, 0.0));
		flashlightDirection = glm::vec3(rotationMatrix * glm::vec4(flashlightDirection, 1.0));
		m_Shader->SetVec3f("u_Flashlight.direction", flashlightDirection.x, flashlightDirection.y, flashlightDirection.z);
		// Flashlight cutoff angle
		m_Shader->SetFloat("u_Flashlight.cutOff", glm::cos(glm::radians(1.0f)));
		m_Shader->SetFloat("u_Flashlight.outerCutOff", glm::cos(glm::radians(35.0f)));

		// Bind shadow depth map texture to shader
		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_ShadowDepthMap));
		m_Shader->SetInt("shadowMap", 3);

		// Render the first cube
		m_CubeTexture->Bind(1);
		m_Shader->SetInt("u_Material.diffuse", 1);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_Shader);

		// Change model matrix and render the second cube
		m_Shader->SetMatrix4f("model", lightModelMatrix2);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_Shader);

		// Set any different uniforms for the ground
		modelMatrix = glm::mat4(1.0);
		m_Shader->SetMatrix4f("model", modelMatrix);
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
		m_Shader->SetVec3f("u_Material.specular", 0.1f, 0.1f, 0.1f);
		m_Shader->SetFloat("u_Material.shininess", 4.0f);

		// Directional light
		m_Shader->SetVec3("u_DirLight.direction", m_DirLightDirection);
		m_Shader->SetVec3("u_DirLight.ambient", m_DirLightAmbient);
		m_Shader->SetVec3("u_DirLight.diffuse", m_DirLightDiffuse);
		m_Shader->SetVec3("u_DirLight.specular", m_DirLightSpecular); 

		// Generate framebuffer for shadow mapping
		glGenFramebuffers(1, &m_DepthMapFBO);
		// 2D buffer for storing depth values
		glGenTextures(1, &m_ShadowDepthMap);
		glBindTexture(GL_TEXTURE_2D, m_ShadowDepthMap);
		// We only need the depth information when rendering the scene from the light's perspective, so no need for a colour or stencil buffer
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// Attach the shadow map buffer to the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowDepthMap, 0);
		//A framebuffer is not 'complete' without a colour buffer so we need to explicitly set OpenGL to not render any colour data
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		// Reset to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

