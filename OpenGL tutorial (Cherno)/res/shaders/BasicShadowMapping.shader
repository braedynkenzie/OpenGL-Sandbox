#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TextureCoords;
layout(location = 2) in vec3 a_Normal;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {

	// TODO: should pass this as a uniform to optimize (costly to perform matrix inverse in shaders)
	mat3 normalMatrix = mat3(transpose(inverse(model)));
	Normal = normalMatrix * a_Normal;
	
	// Want to pass FragPosition for lighting purposes, which is in world coordinates
	FragPosition = (model * vec4(a_Position, 1.0)).xyz;

	TexCoords = a_TextureCoords;

	gl_Position = proj * view * model * vec4(a_Position, 1.0);
}


#shader fragment
#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPosition;

out vec4 FragColour;

uniform vec3 viewPos;

struct Material {
	// Ambient not necessary when using a diffuse map
	// vec3 ambient;
	// Replace vec3 diffuse with sampler2D type
	// vec3 diffuse;
	sampler2D diffuse;
	vec3 specular;
	float shininess;
};
uniform Material u_Material;

struct PointLight {
	bool isActive;
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	// Implementing attenuation: f_att = 1.0 / (constant + linear*distance + quadratic*distance^2)
	float constant;
	float linear;
	float quadratic;
};
#define MAX_NUM_POINT_LIGHTS 100
uniform int numPointLights;
uniform PointLight pointLights[MAX_NUM_POINT_LIGHTS];

struct SpotLight {
	bool on;
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	// Attenuation variables
	float constant;
	float linear;
	float quadratic;

	// Angle of spotlight
	float cutOff;
	float outerCutOff;
};
uniform SpotLight u_Flashlight;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight u_DirLight;

// Function declarations
vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, bool blinnPhongEnabled);
vec3 CalcSpotLight(vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirLight u_DirLight, vec3 normal, vec3 viewDir);

void main() {

	// Phong lighting (using directional, point lights, spotlights)
	//
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPosition);
	vec3 result = vec3(0.0f);

	// Directional lighting
	result += CalcDirLight(u_DirLight, norm, viewDir);

	// Point lights
	//for (int i = 0; i < numPointLights; i++) {
	//	if (pointLights[i].isActive)
	//		result += CalcPointLight(pointLights[i], norm, FragPosition, viewDir, true);
	//}

	// Flashlight (spot light)
	//result += CalcSpotLight(norm, FragPosition, viewDir);

	// Gamma correction
	float gamma = 2.2;
	result = pow(result, vec3(1.0 / gamma));

	// Set fragment colour to combined result
	FragColour = vec4(result, 1.0);
}


vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir)
{
	// Ambient
	vec3 ambient = dirLight.ambient * vec3(texture(u_Material.diffuse, TexCoords));
	//
	// Diffuse 
	vec3 lightDir = dirLight.direction;
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = dirLight.diffuse * diff * vec3(texture(u_Material.diffuse, TexCoords));
	//
	float spec = 0.0;
	// Blinn-Phong model specular 
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);
	vec3 specular = dirLight.specular * spec * u_Material.specular;

	// Combine 
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, bool blinnPhongEnabled)
{
	// Ambient
	vec3 ambient = pointLight.ambient * vec3(texture(u_Material.diffuse, TexCoords));
	//
	// Diffuse 
	vec3 lightDir = normalize(pointLight.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = pointLight.diffuse * diff * vec3(texture(u_Material.diffuse, TexCoords));
	//
	float spec = 0.0;
	if (blinnPhongEnabled) {
		// Blinn-Phong model specular correction
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);
	} else {
		// Specular (regular Phong model style)
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	}
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

vec3 CalcSpotLight(vec3 norm, vec3 FragPos, vec3 viewDir)
{
	// Flashlight
	// 
	vec3 fl_ambient = vec3(0.0f);
	vec3 fl_diffuse = vec3(0.0f);
	vec3 fl_specular = vec3(0.0f);
	// Compare light angle to cutOff
	if (u_Flashlight.on) {
		vec3 flashlightDir = normalize(u_Flashlight.position - FragPos);
		// flashlight ambient
		fl_ambient = u_Flashlight.ambient * vec3(texture(u_Material.diffuse, TexCoords));
		// flashlight diffuse 
		float fl_diff = max(0.0, dot(norm, flashlightDir));
		fl_diffuse = u_Flashlight.diffuse * fl_diff * vec3(texture(u_Material.diffuse, TexCoords));
		// flashlight specular 
		float fl_shininess = 16;
		vec3 fl_reflectDir = reflect(-flashlightDir, norm);
		float fl_spec = pow(max(dot(viewDir, fl_reflectDir), 0.0), u_Material.shininess);
		fl_specular = (fl_spec * u_Material.specular) * u_Flashlight.specular;
		// Flashlight attenuation
		float flashlightDistance = length(u_Flashlight.position - FragPos);
		float fl_attenuation = 2.6 / (u_Flashlight.constant + u_Flashlight.linear * flashlightDistance + u_Flashlight.quadratic * (flashlightDistance * flashlightDistance));
		fl_ambient *= fl_attenuation;
		fl_diffuse *= fl_attenuation;
		fl_specular *= fl_attenuation;
		// Smooth flashlight edge transition
		float theta = dot(flashlightDir, normalize(-u_Flashlight.direction));
		float epsilon = u_Flashlight.cutOff - u_Flashlight.outerCutOff;
		float fl_intensity = clamp((theta - u_Flashlight.outerCutOff) / epsilon, 0.0, 1.0);
		fl_ambient *= fl_intensity;
		fl_diffuse *= fl_intensity;
		fl_specular *= fl_intensity;
	}
	// Combine 
	return (fl_ambient + fl_diffuse + fl_specular);
}