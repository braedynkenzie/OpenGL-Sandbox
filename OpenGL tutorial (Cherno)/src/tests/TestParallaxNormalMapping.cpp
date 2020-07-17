#include "TestParallaxNormalMapping.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackParallaxNormalMapping(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackParallaxNormalMapping(GLFWwindow* window, double xOffset, double yOffset);
	void processInputParallaxNormalMapping(GLFWwindow* window);
	void mouse_button_callbackParallaxNormalMapping(GLFWwindow* window, int button, int action, int mods);
	void CalculateTangentBitangent(glm::vec3& tangent, glm::vec3& bitangent, glm::vec3& edge1, glm::vec3& edge2, glm::vec2& deltaUV1, glm::vec2& deltaUV2);

	// Init static variable
	TestParallaxNormalMapping* TestParallaxNormalMapping::instance;

	TestParallaxNormalMapping::TestParallaxNormalMapping(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 10.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_QuadParallaxShader(new Shader("res/shaders/ParallaxNormalMapping.shader")),
		m_VA_Quad(new VertexArray()),
		m_QuadTexture0(new Texture("res/textures/bricks_texture_parallax.png", true, false)),
		m_QuadNormalMap0(new Texture("res/textures/bricks_normal_parallax.png", true, false)),
		m_QuadHeightMap0(new Texture("res/textures/bricks_heightmap_parallax.png", true, false)),
		m_QuadTexture1(new Texture("res/textures/wooden_floor_texture.png", true, false)),
		m_QuadNormalMap1(new Texture("res/textures/parallax_indents_normal_map.png", true, false)),
		m_QuadHeightMap1(new Texture("res/textures/parallax_indents_depth_map.png", true, false)),
		m_UsingParallaxMapping(true),
		m_ParallaxHeightScale(0.1f),
		m_ActiveTextureIndex(1)
	{
		instance = this;

		// Quad data
		glm::vec3 pos1(-0.5, -0.5, 0.5);
		glm::vec3 pos2( 0.5,  0.5, 0.5);
		glm::vec3 pos3(-0.5,  0.5, 0.5);
		glm::vec3 pos4( 0.5, -0.5, 0.5);
		glm::vec2 uv1(0.0, 0.0);
		glm::vec2 uv2(1.0, 1.0);
		glm::vec2 uv3(0.0, 1.0);
		glm::vec2 uv4(1.0, 0.0);
		glm::vec3 nm(0.0, 0.0, 1.0);
		// Calculate tangent/bitangent vectors
		// Triangle 1
		glm::vec3 tangent1;
		glm::vec3 bitangent1;
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;
		CalculateTangentBitangent(tangent1, 
								  bitangent1, 
								  edge1, 
								  edge2, 
								  deltaUV1, 
								  deltaUV2);

		float quadVertices[] = {
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x,  uv1.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x,  uv2.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x,  uv3.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x,  uv4.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z
		};

		unsigned int quadIndices[]{
			0, 1, 2,
			3, 1, 0
		};

		// Quad Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Quad = new VertexBuffer(quadVertices, 14 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout quadVBLayout;
		quadVBLayout.Push<float>(3); // Vertex position,	 vec3
		quadVBLayout.Push<float>(3); // Normals,			 vec3
		quadVBLayout.Push<float>(2); // Texture coordinates, vec2
		quadVBLayout.Push<float>(3); // Tangents,			 vec3
		quadVBLayout.Push<float>(3); // Bitangents,			 vec3
		m_VA_Quad->AddBuffer(*m_VB_Quad, quadVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Quad = new IndexBuffer(quadIndices, 6);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestParallaxNormalMapping::~TestParallaxNormalMapping()
	{
	}

	void TestParallaxNormalMapping::ToggleParallaxMapping(const bool flag)
	{
		m_UsingParallaxMapping = flag;
		m_QuadParallaxShader->SetBool("u_UsingParallaxMapping", m_UsingParallaxMapping);
	}

	void TestParallaxNormalMapping::ParallaxHeightScaling(int dir)
	{
		if (dir == 1)
		{
			m_ParallaxHeightScale *= 1.01f;
		}
		else if (dir == -1)
		{
			m_ParallaxHeightScale *= 0.99f;
		}
	}

	void TestParallaxNormalMapping::SwitchTexture(int texIndex)
	{
		m_QuadParallaxShader->Bind();
		switch (texIndex)
		{
			case 0: 
				m_ActiveTextureIndex = 0;
				// Update texture uniform
				m_QuadTexture0->Bind(1);
				// Update normal map
				m_QuadNormalMap0->Bind(2);
				// Update height map
				m_QuadHeightMap0->Bind(3);
				break;

			case 1:
				m_ActiveTextureIndex = 1;
				// Update texture uniform
				m_QuadTexture1->Bind(1);
				// Update normal map
				m_QuadNormalMap1->Bind(2);
				// Update height map
				m_QuadHeightMap1->Bind(3);
				break;

			default:
				ASSERT(0); // Should never get here
		}
	}

	void TestParallaxNormalMapping::OnUpdate(float deltaTime)
	{
	}

	void TestParallaxNormalMapping::OnRender()
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
		processInputParallaxNormalMapping(m_MainWindow);

		Renderer renderer;
		// Set per-frame uniforms
		m_QuadParallaxShader->Bind();
		// Model, View, Projection matrices
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f));
		//modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_QuadParallaxShader->SetMatrix4f("model", modelMatrix);
		m_QuadParallaxShader->SetMatrix4f("view", viewMatrix);
		m_QuadParallaxShader->SetMatrix4f("projection", projMatrix);
		m_QuadParallaxShader->SetVec3("viewPos", m_Camera.Position);
		//m_QuadParallaxShader->SetVec3("lightPos", ...);
		m_QuadParallaxShader->SetFloat("height_scale", m_ParallaxHeightScale);
		
		renderer.DrawTriangles(*m_VA_Quad, *m_IB_Quad, *m_QuadParallaxShader);
	}

	void TestParallaxNormalMapping::OnImGuiRender()
	{
		// ImGui interface
		if(m_UsingParallaxMapping)
			ImGui::Text("PRESS '3' to turn off Parallax Mapping");
		else
			ImGui::Text("PRESS '4' to turn on Parallax Mapping");
		ImGui::Text("PRESS '5' to increase height scaling");
		ImGui::Text("PRESS '6' to decrease height scaling");
		ImGui::Text(" - - - ");
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestParallaxNormalMapping::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//  Reset all uniforms
		m_QuadParallaxShader->Bind();
		m_QuadParallaxShader->SetBool("u_UsingParallaxMapping", m_UsingParallaxMapping);
		// Bind all texture maps
		SwitchTexture(m_ActiveTextureIndex);
		// Diffuse texture
		m_QuadParallaxShader->SetInt("diffuseMap", 1);
		// Normal map
		m_QuadParallaxShader->SetInt("normalMap", 2);
		// Height map for Parallax texture
		m_QuadParallaxShader->SetInt("depthMap", 3);


		
		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackParallaxNormalMapping);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackParallaxNormalMapping);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackParallaxNormalMapping);
	}

	void scroll_callbackParallaxNormalMapping(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestParallaxNormalMapping* parallaxNormalMappingTest = test::TestParallaxNormalMapping::GetInstance();
		Camera* parallaxNormalMappingCamera = parallaxNormalMappingTest->GetCamera();
		parallaxNormalMappingCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackParallaxNormalMapping(GLFWwindow* window, double xpos, double ypos)
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

		test::TestParallaxNormalMapping* parallaxNormalMappingTest = test::TestParallaxNormalMapping::GetInstance();
		Camera* parallaxNormalMappingCamera = parallaxNormalMappingTest->GetCamera();
		parallaxNormalMappingCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackParallaxNormalMapping(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestParallaxNormalMapping* parallaxNormalMappingTest = test::TestParallaxNormalMapping::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputParallaxNormalMapping(GLFWwindow* window) {
		test::TestParallaxNormalMapping* parallaxNormalMappingTest = test::TestParallaxNormalMapping::GetInstance();
		Camera* parallaxNormalMappingCamera = parallaxNormalMappingTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			parallaxNormalMappingCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			parallaxNormalMappingCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			parallaxNormalMappingCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			parallaxNormalMappingCamera->ProcessKeyboard(RIGHT, deltaTime);
		// Toggle parallax mapping
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			parallaxNormalMappingTest->ToggleParallaxMapping(false);
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			parallaxNormalMappingTest->ToggleParallaxMapping(true);
		// Change Parallax Height Scaling
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
			parallaxNormalMappingTest->ParallaxHeightScaling(1);
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
			parallaxNormalMappingTest->ParallaxHeightScaling(-1);
		// Toggle between textures
		if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
			parallaxNormalMappingTest->SwitchTexture(1);
		if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
			parallaxNormalMappingTest->SwitchTexture(0);


		
	}

	// Fill in 'tangent' and 'bitangent' vectors given the edge and deltaUV values
	void CalculateTangentBitangent(glm::vec3& tangent, glm::vec3& bitangent, glm::vec3& edge1, glm::vec3& edge2, glm::vec2& deltaUV1, glm::vec2& deltaUV2)
	{
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	}
}

