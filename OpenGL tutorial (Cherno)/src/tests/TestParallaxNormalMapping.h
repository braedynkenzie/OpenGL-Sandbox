#pragma once

#include "Test.h"

#include "Shader.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Camera.h"

namespace test
{
	class TestParallaxNormalMapping : public Test
	{
	private:
		static TestParallaxNormalMapping* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_CameraPos;
		Camera m_Camera;
		Shader* m_QuadParallaxShader;
		VertexArray* m_VA_Quad;
		VertexBuffer* m_VB_Quad;
		IndexBuffer* m_IB_Quad;
		Texture* m_QuadTexture0;
		Texture* m_QuadNormalMap0;
		Texture* m_QuadHeightMap0;
		Texture* m_QuadTexture1;
		Texture* m_QuadNormalMap1;
		Texture* m_QuadHeightMap1;
		bool m_UsingParallaxMapping;
		float m_ParallaxHeightScale;
		int m_ActiveTextureIndex;
	public:
		TestParallaxNormalMapping(GLFWwindow*& mainWindow);
		~TestParallaxNormalMapping();

		void ToggleParallaxMapping(const bool flag);
		void ParallaxHeightScaling(int dir);
		void SwitchTexture(int texIndex);

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestParallaxNormalMapping* GetInstance() { return instance; }

	};
}
