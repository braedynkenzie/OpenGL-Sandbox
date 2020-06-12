#pragma once

#include <GL\glew.h>

// Macros
#define ASSERT(x) if (!(x)) { __debugbreak(); }
#ifdef _DEBUG // if running in debug mode
    #define GLCall(x) GLClearError(); x; ASSERT(GLCheckErrors(#x, __FILE__, __LINE__))
#else // else if running in release mode
    #define GLCall(x) x
#endif

void GLClearError();
bool GLCheckErrors(const char* function, const char* file, int line);