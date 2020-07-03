#shader vertex
#version 330 core 

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;

out vec3 v_Normal;
out vec2 v_TexCoords;
out vec3 v_Position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main()
{
    v_Normal = mat3(transpose(inverse(modelMatrix))) * aNormal; // TODO pass as uniform to optimize
    v_TexCoords = aTexCoords;
    v_Position = vec3(modelMatrix * vec4(aPos, 1.0));
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(v_Position, 1.0);
};

#shader fragment
#version 330 core

out vec4 FragColour;

in vec3 v_Normal;
in vec2 v_TexCoords;
in vec3 v_Position;

uniform vec3 u_CameraPos;
uniform sampler2D u_SurfaceTexture;
uniform samplerCube u_SkyboxTexture;

void main()
{
    vec3 incomingAngle = normalize(v_Position - u_CameraPos);
    vec3 reflectionAngle = reflect(incomingAngle, normalize(v_Normal));
    vec4 surfaceColour = texture(u_SurfaceTexture, v_TexCoords);
    vec4 envMapColour = vec4(texture(u_SkyboxTexture, reflectionAngle).rgb, 1.0);
    // 'reflectiveAmount' below makes the surface more reflective where it is more bright
    float reflectiveAmount = (surfaceColour.r + surfaceColour.g + surfaceColour.b) / 3.0f;
    if (reflectiveAmount > 0.5f && reflectiveAmount < 0.55f)
        reflectiveAmount *= 0.5;
    else if (reflectiveAmount < 0.5f)
        reflectiveAmount *= 0.1f;
    FragColour = vec4(surfaceColour.rgb * (1 - reflectiveAmount), 1.0f) + (envMapColour * reflectiveAmount);
};