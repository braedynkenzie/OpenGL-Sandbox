#pragma once

#include <functional>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include "GLFW\glfw3.h"

#include "imgui\imgui.h"


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
	};

	class TestMenu : public Test
	{
	private:
		Test*& m_CurrentTest;
		std::vector<std::pair<std::string, test::Test*>> m_Tests;
		GLFWwindow* m_MainWindow;

	public:
		TestMenu(Test*& activeTestPtr, GLFWwindow* mainWindow);

		void OnImGuiRender() override;

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



