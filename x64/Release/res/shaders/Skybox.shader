#shader vertex
#version 330 core 

layout(location = 0) in vec3 aPos;

out vec3 v_TexCoords;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
    v_TexCoords = aPos;
    vec4 pos = projMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
};


#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 v_TexCoords;

uniform samplerCube u_SkyboxTexture;

void main()
{
    FragColor = texture(u_SkyboxTexture, v_TexCoords);
};