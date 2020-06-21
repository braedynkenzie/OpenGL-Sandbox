#shader vertex
#version 330 core 

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoords;

out vec2 v_TexCoords;

uniform mat4 u_MVP;

void main()
{
    v_TexCoords = texCoords;
    gl_Position = u_MVP * position;
};


#shader fragment
#version 330 core

uniform vec4 u_Color;
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;
uniform int u_ActiveTexture;

in vec2 v_TexCoords;

out vec4 color;

void main()
{
    vec4 texColour;
    if (u_ActiveTexture == 0)
    {
        texColour = texture(u_Texture0, v_TexCoords);
    }
    else if (u_ActiveTexture == 1)
    {
        texColour = texture(u_Texture1, v_TexCoords);
    }
    color = texColour;
};