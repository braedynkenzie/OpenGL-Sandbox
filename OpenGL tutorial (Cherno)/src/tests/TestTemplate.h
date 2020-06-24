#pragma once

#include "Test.h"

namespace test
{
	class TestTemplate : public Test
	{
	private:
		GLFWwindow* m_MainWindow;

	public: 
		TestTemplate(GLFWwindow*& mainWindow);
		~TestTemplate();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

	};
}
