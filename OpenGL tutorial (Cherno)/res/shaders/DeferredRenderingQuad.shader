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

struct PointLight {
    vec3 Position;
    vec3 Colour;

    float Linear;
    float Quadratic;
};
const int NUM_POINTLIGHTS = 32;
uniform PointLight pointLights[NUM_POINTLIGHTS];

void main()
{
    // Retrieve all the fragment data from the GBuffer
    vec3 FragPos            = texture(gPosition,    v_TexCoords).rgb;
    vec3 Normal             = texture(gNormal,      v_TexCoords).rgb;
    vec4 albedoSpecSample   = texture(gAlbedoSpec,  v_TexCoords);
    vec3 Diffuse            = albedoSpecSample.rgb; // first three components 
    float Specular          = albedoSpecSample.a; // fourth component

    // Then use to calculate the lighting as usual
    vec3 lighting = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - FragPos);
    for (int i = 0; i < NUM_POINTLIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(pointLights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pointLights[i].Colour;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = pointLights[i].Colour * spec * Specular;
        // attenuation
        float distance = length(pointLights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + pointLights[i].Linear * distance + pointLights[i].Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    FragColour = vec4(lighting, 1.0);


    // Testing
    //FragColour = vec4(Diffuse, 1.0);
    //FragColour = vec4(vec3(Specular), 1.0);
    //FragColour = vec4(FragPos, 1.0);
    //FragColour = vec4(Diffuse, Specular);
    //FragColour = vec4(Normal, 1.0);
}