#pragma once

#include "Test.h"

namespace test
{
	class TestClearColour : public Test
	{
	private:
		static float m_ClearColour[4];
		// todo delete these
		GLFWwindow* m_MainWindow;

	public: 
		TestClearColour(GLFWwindow*& mainWindow);
		~TestClearColour();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		static float* GetClearColour() { return m_ClearColour; }
	};
}
