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
	void mouse_button_callbackPhongTest(GLFWwindow* window, int button, int action, int mods);
	void processMovingLights(std::vector<PointLight>& pointLights, float deltaTime);
	void SetupPointLights(Shader* pointLightsShader, Shader* groundShader, const std::vector<PointLight>& pointLights, const glm::vec3& diffuseIntensity, const glm::vec3& ambientIntensity,
		const glm::vec3& specularIntensity, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, Renderer renderer, VertexArray* VA_PointLight, IndexBuffer* IB_PointLight);

	// Init static variable
	TestPhongLighting* TestPhongLighting::instance;

	TestPhongLighting::TestPhongLighting(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow), 
		m_PointLights(std::vector<PointLight>()),
		m_GroundShader(new Shader("res/shaders/BasicPhongModel.shader")),
	    m_PointLightsShader(new Shader("res/shaders/PointLights.shader")),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)), 
		m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), 
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)), 
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_IsFlashlightOn(true),
		m_FlashlightColour(glm::vec3(1.0f)), m_FlashlightDiffuseIntensity(glm::vec3(1.0f)),
		m_FlashlightAmbientIntensity(glm::vec3(0.4f)), m_FlashlightSpecularIntensity(glm::vec3(0.2f)),
		m_FlashlightDiffuseColour( m_FlashlightColour * m_FlashlightDiffuseIntensity), 
		m_FlashlightAmbientColour(m_FlashlightDiffuseColour * m_FlashlightAmbientIntensity),
		m_FloatingLightColour(glm::vec3(1.0, 1.0, 1.0)),
		m_FloatingLightPos(glm::vec3(2.0f, 2.0f, -40.0f)),
		m_FloatingLightDiffuseIntensity(glm::vec3(1.0f)), m_FloatingLightAmbientIntensity(glm::vec3(0.05f)), 
		m_FloatingLightSpecularIntensity(glm::vec3(0.1f)),
		m_FloatingLightDiffuseColour(),
		m_FloatingLightAmbientColour(),
		m_BlinnPhongEnabled(true),
		m_WoodenGroundEnabled(true),
		// Skybox data
		m_SkyboxShader(new Shader("res/shaders/Skybox.shader")),
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

		// Skybox Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Skybox = new VertexBuffer(skyboxVertices, 3 * 4 * 6 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout skyboxVBLayout;
		skyboxVBLayout.Push<float>(3); // Vertex positions,  vec3
		m_VA_Skybox->AddBuffer(*m_VB_Skybox, skyboxVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Skybox = new IndexBuffer(skyboxIndices, 6 * 6);

		// Ground Vertex Array setup
		m_VA_Ground = std::make_unique<VertexArray>();
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Ground = std::make_unique<VertexBuffer>(groundVertices, 8 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout groundVBLayout;
		groundVBLayout.Push<float>(3); // Vertex position,vec3
		groundVBLayout.Push<float>(2); // Texture coordinates, vec2
		groundVBLayout.Push<float>(3); // Normals, vec3
		m_VA_Ground->AddBuffer(*m_VB_Ground, groundVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Ground = std::make_unique<IndexBuffer>(groundIndices, 6);

		// Pointlight Vertex Array setup
		m_VA_PointLight = new VertexArray();
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_PointLight = new VertexBuffer(pointLightVertices, 5 * 8 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout pointLightVBLayout;
		pointLightVBLayout.Push<float>(3); // Vertex position,vec3
		pointLightVBLayout.Push<float>(2); // Texture coordinates, vec2
		m_VA_PointLight->AddBuffer(*m_VB_PointLight, pointLightVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_PointLight = new IndexBuffer(pointLightIndices, 6 * 6);

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

		// Calculate all point light projectile movements
		processMovingLights(m_PointLights, deltaTime);

		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 10.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor, 
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Point light changing colour
		m_FloatingLightColour.x = sin(glfwGetTime() * 1.0f) / 2.0f + 0.7f;
		m_FloatingLightColour.y = sin(glfwGetTime() * 0.5f) / 2.0f + 0.7f;
		m_FloatingLightColour.z = sin(glfwGetTime() * 0.4f) / 2.0f + 0.7f;
		// Set floating point lights' changing colour
		for (int i = 0; i < 3; i++)
		{
			PointLight* pointLight = &m_PointLights[i];
			pointLight->Colour = glm::vec3(m_FloatingLightColour.x + cos(i - 0.4), m_FloatingLightColour.y + sin(i - 0.4), m_FloatingLightColour.z + sin(i - 0.4));
		}

		Renderer renderer;

		// Bind shader and set any 'per frame' uniforms
		m_GroundShader->Bind();
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(50.0, 0.0, 36.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 800.0f);
		m_GroundShader->SetMatrix4f("model", modelMatrix);
		m_GroundShader->SetMatrix4f("view", viewMatrix);
		m_GroundShader->SetMatrix4f("proj", projMatrix);

		// Update camera's viewing position each frame
		m_GroundShader->SetVec3f("viewPos", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);

		// Check/set Blinn-Phong mode
		m_GroundShader->SetBool("u_BlinnPhongEnabled", m_BlinnPhongEnabled);

		// Flashlight's properties
		//
		m_GroundShader->SetBool("u_Flashlight.on", m_IsFlashlightOn);
		m_GroundShader->SetVec3("u_Flashlight.ambient", m_FlashlightAmbientColour);
		m_GroundShader->SetVec3("u_Flashlight.diffuse", m_FlashlightDiffuseColour);
		m_GroundShader->SetVec3("u_Flashlight.specular", m_FlashlightSpecularIntensity);
		// Flashlight attenuation properties
		m_GroundShader->SetFloat("u_Flashlight.constant", 1.0f);
		m_GroundShader->SetFloat("u_Flashlight.linear", 0.02f);
		m_GroundShader->SetFloat("u_Flashlight.quadratic", 0.01f);
		// Flashlight position and direction
		glm::vec3 flashlightPosition = m_Camera.Position;
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0), 2.0f * m_Camera.Right);
		flashlightPosition = glm::vec3(translationMatrix * glm::vec4(flashlightPosition, 1.0));
		m_GroundShader->SetVec3f("u_Flashlight.position", flashlightPosition.x, flashlightPosition.y, flashlightPosition.z);
		glm::vec3 flashlightDirection = m_Camera.Front;
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(4.0f), glm::vec3(0.0, 1.0, 0.0));
		flashlightDirection = glm::vec3(rotationMatrix * glm::vec4(flashlightDirection, 1.0));
		m_GroundShader->SetVec3f("u_Flashlight.direction", flashlightDirection.x, flashlightDirection.y, flashlightDirection.z);
		// Flashlight cutoff angle
		m_GroundShader->SetFloat("u_Flashlight.cutOff", glm::cos(glm::radians(1.0f)));
		m_GroundShader->SetFloat("u_Flashlight.outerCutOff", glm::cos(glm::radians(35.0f)));
		//
		// Set all point light uniforms and render them
		SetupPointLights(m_PointLightsShader, m_GroundShader, m_PointLights, m_FloatingLightDiffuseIntensity, m_FloatingLightAmbientIntensity, m_FloatingLightSpecularIntensity, viewMatrix, projMatrix, renderer, m_VA_PointLight, m_IB_PointLight);
		// Render ground
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_GroundShader); 

		// Then render the skybox with depth testing at LEQUAL (and set z component to be (w / w) = 1.0 = max depth in vertex shader)
		glDepthFunc(GL_LEQUAL);
		m_SkyboxShader->Bind();
		// Model, View, Projection matrices
		modelMatrix = glm::mat4(1.0f);
		viewMatrix = glm::mat4(glm::mat3(m_Camera.GetViewMatrix())); // Gets rid of any translation
		projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 800.0f);
		m_SkyboxShader->SetMatrix4f("modelMatrix", modelMatrix);
		m_SkyboxShader->SetMatrix4f("viewMatrix", viewMatrix);
		m_SkyboxShader->SetMatrix4f("projMatrix", projMatrix);
		renderer.DrawTriangles(*m_VA_Skybox, *m_IB_Skybox, *m_SkyboxShader);
		glDepthFunc(GL_LESS);
	}

	void processMovingLights(std::vector<PointLight>& pointLights, float deltaTime)
	{
		for (PointLight& pointLight : pointLights)
		{
			// Update positions
			glm::vec3 newPosition = pointLight.Position;
			newPosition += (pointLight.Direction * pointLight.Speed);
			//newPosition *= deltaTime; // TODO 
			pointLight.Position = newPosition;

			if (pointLight.Position.y > -9.4)
			{
				// Projectile is in the air
				pointLight.Direction.y -= 0.04;
			}
			else if (pointLight.Position.y <= -9.4)
			{
				// Projectile is on the ground
				pointLight.Speed *= 0.7;
				// Check y velocity
				float yVelocity = pointLight.Direction.y * pointLight.Speed;
				if (yVelocity < -0.4)
				{
					// Bouncing
					pointLight.Direction.y = -0.5 * pointLight.Direction.y;
					pointLight.Position.y = -9.3;
				}
				else
				{
					// Sliding along ground
					pointLight.Direction.y = 0;
					pointLight.Position.y = -9.4;
					if (pointLight.Speed < 0.02)
						pointLight.Speed = 0;
				}
			}
		}
	}

	void SetupPointLights(Shader* pointLightsShader, Shader* groundShader, const std::vector<PointLight>& pointLights, const glm::vec3& diffuseIntensity, const glm::vec3& ambientIntensity,
		const glm::vec3& specularIntensity, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, Renderer renderer, VertexArray* VA_PointLight, IndexBuffer* IB_PointLight)
	{
		for (int i = 0; i < pointLights.size(); i++)
		{
			PointLight pointLight = pointLights[i];
			// Point light properties in m_PointLightsShader
			glm::vec3 floatingLightDiffuseColour = pointLight.Colour * diffuseIntensity;
			glm::vec3 floatingLightAmbientColour = floatingLightDiffuseColour * ambientIntensity;
			pointLightsShader->Bind();
			// Model matrix: Translate and scale the light object
			glm::mat4 pointLightsModelMatrix = glm::mat4(1.0f);
			pointLightsModelMatrix = glm::translate(pointLightsModelMatrix, pointLight.Position);
			pointLightsModelMatrix = glm::scale(pointLightsModelMatrix, glm::vec3(1.0f));
			pointLightsShader->SetMatrix4f("model", pointLightsModelMatrix);
			pointLightsShader->SetMatrix4f("view", viewMatrix);
			pointLightsShader->SetMatrix4f("proj", projMatrix);
			// Light colour uniform
			pointLightsShader->SetVec3("pointLightColour", pointLight.Colour * 1.2f);
			//
			// Render call for each pointlight
			renderer.DrawTriangles(*VA_PointLight, *IB_PointLight, *pointLightsShader);
			
			// Point light properties in m_GroundShader
			groundShader->Bind();
			std::string index = std::to_string(i);
			groundShader->SetBool("pointLights[" + index + "].isActive", true);
			groundShader->SetVec3("pointLights[" + index + "].ambient", floatingLightAmbientColour);
			groundShader->SetVec3("pointLights[" + index + "].diffuse", floatingLightDiffuseColour);
			groundShader->SetVec3("pointLights[" + index + "].specular", specularIntensity);
			// Point light attenuation properties 
			groundShader->SetFloat("pointLights[" + index + "].constant", 0.2f);
			groundShader->SetFloat("pointLights[" + index + "].linear", 0.01f);
			groundShader->SetFloat("pointLights[" + index + "].quadratic", 0.004f);
			// Point light position
			groundShader->SetVec3("pointLights[" + index + "].position", pointLight.Position);
		}

	}

	void TestPhongLighting::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("LEFT CLICK to add light sources");
		if (!m_BlinnPhongEnabled)
			ImGui::Text("PRESS 3: Turn ON Blinn-Phong specular");
		else
			ImGui::Text("PRESS 4: Turn OFF Blinn-Phong specular");
		if (!m_WoodenGroundEnabled)
			ImGui::Text("PRESS 5: Wooden flooring");
		else
			ImGui::Text("PRESS 6: Rocky ground");
		ImGui::Text(" - - - ");
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestPhongLighting::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Clear all pointlights and then add floating light source back
		m_PointLights.clear();
		PointLight floatingLight1 = { m_FloatingLightColour, m_FloatingLightPos + glm::vec3(0.0, 4.0, 0.0), glm::vec3(0.0), 0.0 };
		m_PointLights.push_back(floatingLight1);
		PointLight floatingLight2 = { m_FloatingLightColour, m_FloatingLightPos + glm::vec3(4.0, -8.0, -60.0), glm::vec3(0.0), 0.0 };
		m_PointLights.push_back(floatingLight2);
		PointLight floatingLight3 = { m_FloatingLightColour, m_FloatingLightPos + glm::vec3(-6.0, -5.0, -120.0), glm::vec3(0.0), 0.0 };
		m_PointLights.push_back(floatingLight3);

		// Bind shader programs and set uniforms
		m_GroundShader->Bind();
		m_GroundShader->SetInt("numPointLights", m_PointLights.size());
		m_WoodenGroundTexture = new Texture("res/textures/wooden_floor_texture.png", false);
		m_WoodenGroundTexture->BindAndSetRepeating(1);
		m_RockyGroundTexture = new Texture("res/textures/dirt_ground_texture.png", false);
		m_RockyGroundTexture->BindAndSetRepeating(0);
		if (m_WoodenGroundEnabled)
		{
			m_WoodenGroundTexture->Bind(1);
			m_GroundShader->SetInt("u_MaterialDiffuse", 1); 
		}
		else
		{
			m_RockyGroundTexture->Bind(0);
			m_GroundShader->SetInt("u_MaterialDiffuse", 0);
		}
		m_GroundShader->SetVec3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
		m_GroundShader->SetFloat("u_Material.shininess", 12.0f);
		// Reset MVP matrices on activation
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(50.0, 0.0, 36.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 800.0f);
		m_GroundShader->SetMatrix4f("model", modelMatrix);
		m_GroundShader->SetMatrix4f("view", viewMatrix);
		m_GroundShader->SetMatrix4f("proj", projMatrix);

		// Skybox shader
		delete m_SkyboxTexture;
		m_SkyboxTexture = new Texture(std::vector<std::string>({ "res/textures/night_skybox_alt/right.jpg",
																 "res/textures/night_skybox_alt/left.jpg",
																 "res/textures/night_skybox_alt/top.jpg",
																 "res/textures/night_skybox_alt/bottom.jpg",
																 "res/textures/night_skybox_alt/front.jpg",
																 "res/textures/night_skybox_alt/back.jpg" }));
		m_SkyboxShader->Bind();
		m_SkyboxTexture->BindCubemap(4);
		m_SkyboxShader->SetInt("u_SkyboxTexture", 4);

		// Reset all callbacks
		// Callback function for keyboard inputs
		processInputPhongTest(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackPhongTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackPhongTest);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackPhongTest);
	}

	void TestPhongLighting::NewProjectile()
	{
		m_GroundShader->Bind();
		if (m_PointLights.size() < 100)
		{
			PointLight newPointLight = { m_FloatingLightColour, m_Camera.Position, m_Camera.Front, 3.0 };
			m_PointLights.push_back(newPointLight);
			m_GroundShader->SetInt("numPointLights", m_PointLights.size());
		}
	}

	void TestPhongLighting::ToggleBlinnPhong(bool flag)
	{
		m_BlinnPhongEnabled = flag;
	}

	void TestPhongLighting::ToggleGroundTexture(bool woodenGroundTextureFlag)
	{
		m_GroundShader->Bind();
		if (woodenGroundTextureFlag)
		{
			m_WoodenGroundTexture->Bind(1); 
			m_GroundShader->SetInt("u_MaterialDiffuse", 1);
			m_GroundShader->SetVec3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
			m_GroundShader->SetFloat("u_Material.shininess", 12.0f);
			m_WoodenGroundEnabled = true;
		}
		else
		{
			m_RockyGroundTexture->Bind(0);
			m_GroundShader->SetInt("u_MaterialDiffuse", 0);
			m_GroundShader->SetVec3f("u_Material.specular", 0.0f, 0.1f, 0.0f);
			m_GroundShader->SetFloat("u_Material.shininess", 2.0f);
			m_WoodenGroundEnabled = false;
		}
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

	void mouse_button_callbackPhongTest(GLFWwindow* window, int button, int action, int mods)
	{
		test::TestPhongLighting* lightingTest = test::TestPhongLighting::GetInstance();

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			lightingTest->NewProjectile();
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

		// Toggle Blinn-Phong specular lighting
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			lightingTest->ToggleBlinnPhong(true);
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			lightingTest->ToggleBlinnPhong(false);

		// Toggle ground texture
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
			lightingTest->ToggleGroundTexture(true);
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
			lightingTest->ToggleGroundTexture(false);
	}
}
