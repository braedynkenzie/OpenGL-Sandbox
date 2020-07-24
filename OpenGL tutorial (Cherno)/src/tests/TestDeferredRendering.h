#pragma once

#include "Test.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "Camera.h"

#include <memory>
#include <Model.h>


namespace test
{
	class TestDeferredRendering : public Test
	{
	private:
		static TestDeferredRendering* instance;
		GLFWwindow* m_MainWindow;
		bool modelLoaded;
		Model* m_BackpackModel;
		VertexArray* m_VA_Ground;
		VertexBuffer* m_VB_Ground;
		IndexBuffer* m_IB_Ground;
		VertexArray*  m_VA_Quad;
		VertexBuffer* m_VB_Quad;
		IndexBuffer*  m_IB_Quad;
		Shader* m_GBufferShader;
		Shader* m_QuadShader;
		Texture* m_GroundTexture;
		Texture* m_SecondaryTexture;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		Camera m_Camera;
		// Lighting variables
		const unsigned int NUM_LIGHTS; 
		const int m_NumModelColumns;
		const int m_NumModelRows;
		std::vector<glm::vec3> m_LightPositions;
		std::vector<glm::vec3> m_LightColours;
		float m_SpacingAmount;
		// Deferred Rendering variables
		FrameBuffer* m_GBuffer;
		unsigned int m_RenderBufferID;
		unsigned int m_PositionGBuffer;
		unsigned int m_NormalGBuffer;
		unsigned int m_AlbedoSpecGBuffer;

	public:

		TestDeferredRendering(GLFWwindow*& mainWindow);
		~TestDeferredRendering();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestDeferredRendering* GetInstance() { return instance; }
	};
}
