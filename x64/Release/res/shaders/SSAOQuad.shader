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
uniform sampler2D ssaoTexture;
uniform vec3 viewPos;
uniform vec3 clearColour;

out vec4 FragColour;

struct PointLight {
    vec3 Position;
    vec3 Colour;

    float Linear;
    float Quadratic;
};
const int NUM_POINTLIGHTS = 2;
uniform PointLight pointLights[NUM_POINTLIGHTS];

void main()
{
    // Retrieve all the fragment data from the SSAO GBuffer
    vec3 FragPos            = texture(gPosition,    v_TexCoords).rgb;
    vec3 Normal             = texture(gNormal,      v_TexCoords).rgb;
    vec4 albedoSpecSample   = texture(gAlbedoSpec,  v_TexCoords);
    //vec3 Diffuse            = albedoSpecSample.rgb; 
    vec3 Diffuse            = clearColour; // testing
    //vec3 Diffuse            = vec3(0.95); // testing
    float Specular          = albedoSpecSample.a;
    float AmbientOcclusion = texture(ssaoTexture, v_TexCoords).r;

    // Blinn-Phong lighting model with screen-space ambient occlusion
    vec3 lighting = vec3(0.0);
    vec3 ambient = vec3(Diffuse * AmbientOcclusion * 0.4);
    vec3 viewDir = normalize(-FragPos); // viewpos is (0.0.0)
    for (int i = 0; i < NUM_POINTLIGHTS; ++i)
    {
        // diffuse lighting component
        vec3 lightDir = normalize(pointLights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pointLights[i].Colour;
        // specular lighting component
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 4.0); // shinniness
        vec3 specular = pointLights[i].Colour * spec;
        // attenuation
        float distance = length(pointLights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + pointLights[i].Linear * distance + pointLights[i].Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        // Combine lighting
        lighting += ambient;
        lighting += diffuse;
        lighting += specular;
    }

    FragColour = vec4(lighting, 1.0);





    // Testing
    //FragColour = vec4(AmbientOcclusion * Diffuse, 1.0);
    //FragColour = vec4(vec3(AmbientOcclusion), 1.0);
    //FragColour = vec4(AmbientOcclusion * clearColour, 1.0);
    //FragColour = vec4(Diffuse, 1.0);
    //FragColour = vec4(vec3(Specular), 1.0);
    //FragColour = vec4(FragPos, 1.0);
    //FragColour = vec4(vec3(1.0 + FragPos.z / 6.0), 1.0);
    //FragColour = vec4(Diffuse, Specular);
    //FragColour = vec4(Normal, 1.0);
    //FragColour = vec4(AmbientOcclusion, 1.0);
    //FragColour = vec4(v_TexCoords.x, v_TexCoords.y, 0.0, 1.0);
}