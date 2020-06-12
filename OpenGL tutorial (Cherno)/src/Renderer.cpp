#include "Renderer.h"

#include <iostream>

void GLClearError()
{
    // Goes through and clears any current OpenGL error flags
    while (glGetError() != GL_NO_ERROR);
}

bool GLCheckErrors(const char* function, const char* file, int lineNumber)
{
    while (GLenum errorMsg = glGetError())
    {
        std::cout << "[OpenGL Error code]: " << "(" << errorMsg << ") " << function << ", " << file << ", line: " << lineNumber << std::endl;
        return false;
    }
    return true;
}