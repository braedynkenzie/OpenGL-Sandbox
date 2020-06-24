#pragma once

#include "Test.h"

namespace test
{
	class TestModelLoading : public Test
	{
	private:
		GLFWwindow* m_MainWindow;

	public: 
		TestModelLoading(GLFWwindow*& mainWindow);
		~TestModelLoading();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

	};
}
