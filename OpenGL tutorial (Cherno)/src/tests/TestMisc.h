#pragma once

#include "Test.h"

namespace test
{
	class TestMisc : public Test
	{
	private:
		VertexArray m_VAO;
		Shader m_Shader;
		Renderer m_Renderer;
		glm::vec3 m_ModelTranslation1;
		glm::vec3 m_ModelTranslation2;
		float m_ModelRotationZ;
		float m_ModelRotationY;
		float m_ModelScale;

	public:
		TestMisc(const Renderer& renderer);
		~TestMisc();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	};
}
