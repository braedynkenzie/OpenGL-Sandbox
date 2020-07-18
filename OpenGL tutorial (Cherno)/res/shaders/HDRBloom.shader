#shader vertex
#version 330 core
layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(a_Pos.x, a_Pos.y, 0.0, 1.0);
    v_TexCoords = a_TexCoords;
}

#shader fragment
#version 330 core
out vec4 FragColour;

in vec2 v_TexCoords;

uniform sampler2D framebufferTexture;

void main()
{
    FragColour = texture(framebufferTexture, v_TexCoords);
}