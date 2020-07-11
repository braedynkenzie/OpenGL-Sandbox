#pragma once

#include <functional>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include "GLFW\glfw3.h"

#include "imgui\imgui.h"

#include "Renderer.h"


namespace test
{
	class Test 
	{
	public:
		Test() {}
		virtual ~Test() {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
		virtual void OnActivated() = 0;
	};

	class TestMenu : public Test
	{
	private:
		static TestMenu* instance;
		Test*& m_CurrentTest;
		std::vector<std::pair<std::string, test::Test*>> m_Tests;
		GLFWwindow* m_MainWindow;

	public:
		TestMenu(Test*& activeTestPtr, GLFWwindow* mainWindow);

		void OnImGuiRender() override;
		void OnActivated() override;

		static TestMenu* GetInstance() { return instance; }
		void SetScreenDimensions(unsigned int width, unsigned int height);

		template<typename T>
		void RegisterTestLambda(const std::string& testName, GLFWwindow* window)
		{
			std::cout << "Registering test: " << testName << std::endl;
			m_Tests.push_back(std::make_pair(testName, [&window]() { return new T(window); }));
		}
		
		template<typename T>
			void RegisterTest(const std::string& testName, T test)
		{
			std::cout << "Registering test: " << testName << std::endl;
			m_Tests.push_back(std::make_pair(testName, test));
		}

	};
}



