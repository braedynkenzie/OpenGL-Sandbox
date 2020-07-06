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

void Renderer::DrawTriangles(const VertexArray& VA, const IndexBuffer& IB, const Shader& shader) const
{
    // Bind shader
    shader.Bind();

    // Bind VAO
    // Note this binds the associated VertexBuffer, IndexBuffer, and layout
    VA.Bind();

    // Draw from element buffer
    GLCall(glDrawElements(GL_TRIANGLES, IB.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::DrawPoints(const VertexArray& VA, const IndexBuffer& IB, const Shader& shader) const
{
    // Bind shader
    shader.Bind();

    // Bind VAO
    // Note this binds the associated VertexBuffer, IndexBuffer, and layout
    VA.Bind();

    // Draw from element buffer
    GLCall(glDrawElements(GL_POINTS, IB.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
}
