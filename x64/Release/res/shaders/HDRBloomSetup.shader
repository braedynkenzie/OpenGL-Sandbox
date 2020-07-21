#shader vertex
#version 330 core 

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(location = 2) in vec3 a_Normal;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    vs_out.TexCoords = a_TexCoords;
    vs_out.FragPos = vec3(model * a_Position);
    vs_out.Normal = a_Normal;
    gl_Position = proj * view * model * a_Position;
};


#shader fragment
#version 330 core

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Implementing attenuation: f_att = 1.0 / (constant + linear*distance + quadratic*distance^2)
    float constant;
    float linear;
    float quadratic;
};
#define NUM_POINT_LIGHTS 2
uniform PointLight pointLights[NUM_POINT_LIGHTS];

struct Material {
    // Ambient not necessary when using a diffuse map
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};
uniform Material u_Material;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} fs_in;

uniform vec3 viewPos;

layout(location = 0) out vec4 FragColour;
layout(location = 1) out vec4 BrightColour;

// Function declarations
vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normal;
    normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0f);

    // Messing with normals for buffer cube lighting
    vec3 texColour = vec3(texture(u_Material.diffuse, fs_in.TexCoords));
    float darkness = 1.0 - ((texColour.r + texColour.g + texColour.b) / 3.0);
    normal = normalize(normal + (darkness * darkness * darkness * vec3(0.0, 1.0, 0.0)));

    // Point lights
    for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);
    }

    // Gamma correction
    float gamma = 2.2;
    result = pow(result, vec3(1.0 / gamma));

    // Set fragment colour to combined result
    FragColour = vec4(result, 1.0);

    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColour.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColour = vec4(FragColour.rgb, 1.0); // output colour in Bloom buffer
    else
        BrightColour = vec4(0.0, 0.0, 0.0, 1.0); // black in Bloom colour buffer
};

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Ambient
    vec3 ambient = pointLight.ambient * vec3(texture(u_Material.diffuse, fs_in.TexCoords));
    //
    // Diffuse 
    vec3 lightDir = normalize(pointLight.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = pointLight.diffuse * diff * vec3(texture(u_Material.diffuse, fs_in.TexCoords));
    //
    float spec = 0.0;
    // Specular (a la Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);
    vec3 specular = pointLight.specular * spec * u_Material.specular;

    // Attenuation
    float distance = length(pointLight.position - fragPos);
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Combine 
    return (ambient + diffuse + specular);
}