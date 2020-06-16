#shader vertex
#version 330 core 

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoords;

out vec2 v_TexCoords;

void main()
{
    v_TexCoords = texCoords;
    gl_Position = position;
};


#shader fragment
#version 330 core

uniform vec4 u_Color;
uniform sampler2D u_Texture;

in vec2 v_TexCoords;

out vec4 color;

void main()
{
    vec4 texColour = texture(u_Texture, v_TexCoords);
    color = texColour; // + u_Color;
};