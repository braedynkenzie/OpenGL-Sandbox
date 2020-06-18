#pragma once

#include <functional>
#include <vector>
#include <iostream>

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
		std::vector<std::pair<std::string, std::function<Test*()>>> m_Tests;

	public:
		TestMenu(Test*& activeTestPtr);

		virtual void OnImGuiRender();

		template<typename T> 
		void RegisterTest(const std::string& testName)
		{
			std::cout << "Registering test: " << testName << std::endl;
			m_Tests.push_back(std::make_pair(testName, []() { return new T; }));
		}

	};
}



