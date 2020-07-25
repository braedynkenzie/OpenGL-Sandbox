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

in vec2 v_TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform vec3 viewPos;

out vec4 FragColour;

void main()
{
    // Retrieve all the fragment data from the SSAO GBuffer
    vec3 FragPos            = texture(gPosition,    v_TexCoords).rgb;
    vec3 Normal             = texture(gNormal,      v_TexCoords).rgb;
    vec4 albedoSpecSample   = texture(gAlbedoSpec,  v_TexCoords);
    vec3 Diffuse            = albedoSpecSample.rgb; 
    //float Specular          = albedoSpecSample.a;

    vec3 viewDir = normalize(viewPos - FragPos);


    // Testing
    //FragColour = vec4(Diffuse, 1.0);
    //FragColour = vec4(vec3(Specular), 1.0);
    //FragColour = vec4(FragPos, 1.0);
    FragColour = vec4(vec3(1.0 + FragPos.z / 6.0), 1.0);
    //FragColour = vec4(Diffuse, Specular);
    //FragColour = vec4(Normal, 1.0);
}