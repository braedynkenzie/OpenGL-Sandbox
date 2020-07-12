#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TextureCoords;
layout(location = 2) in vec3 a_Normal;

out VS_OUT{
	vec2 TexCoords;
	vec3 Normal;
	vec3 FragPosition;
	vec4 FragPosLightSpaceOrthographic;
	vec4 FragPosLightSpacePerspective;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 lightSpaceMatrixOrthographic;
uniform mat4 lightSpaceMatrixPerspective;

void main() {

	// TODO: should pass this as a uniform to optimize (costly to perform matrix inverse in shaders)
	mat3 normalMatrix = mat3(transpose(inverse(model)));
	
	vs_out.Normal = normalMatrix * a_Normal;
	vs_out.TexCoords = a_TextureCoords;
	// Pass FragPosition for lighting purposes, which is in world coordinates
	vs_out.FragPosition = (model * vec4(a_Position, 1.0)).xyz;
	// Pass FragPosLightSpace for comparing fragment depths in the shadow map
	vs_out.FragPosLightSpaceOrthographic = lightSpaceMatrixOrthographic * vec4(vs_out.FragPosition, 1.0);
	vs_out.FragPosLightSpacePerspective  = lightSpaceMatrixPerspective  * vec4(vs_out.FragPosition, 1.0);

	gl_Position = proj * view * model * vec4(a_Position, 1.0);
}


#shader fragment
#version 330 core

out vec4 FragColour;

uniform vec3 viewPos;
uniform sampler2D shadowMapOrthographic;
uniform sampler2D shadowMapPerspective;

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

in VS_OUT{
	vec2 TexCoords;
	vec3 Normal;
	vec3 FragPosition;
	vec4 FragPosLightSpaceOrthographic;
	vec4 FragPosLightSpacePerspective;
} fs_in;

// Function declarations
vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, bool blinnPhongEnabled, float shadow);
vec3 CalcSpotLight(vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);
vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir, float shadow);
float ShadowCalculationOrthographic(DirLight dirLight, vec4 FragPosLightSpaceOrthographic, vec3 normal);
float ShadowCalculationPerspective(vec4 FragPosLightSpacePerspective, vec3 normal);

void main() {

	// Phong lighting (using directional, point lights, spotlights)
	//
	vec3 norm = normalize(fs_in.Normal);
	vec3 viewDir = normalize(viewPos - fs_in.FragPosition);
	vec3 result = vec3(0.0f);

	// Shadow mapping calculation
	float orthographicShadow = 1.0 - ShadowCalculationOrthographic(u_DirLight, fs_in.FragPosLightSpaceOrthographic, norm);
	float perspectiveShadow  = 1.0 - ShadowCalculationPerspective(fs_in.FragPosLightSpacePerspective, norm);

	// Directional lighting
	result += CalcDirLight(u_DirLight, norm, viewDir, orthographicShadow);

	// Point lights
	//for (int i = 0; i < numPointLights; i++) {
	//	if (pointLights[i].isActive)
	//		result += CalcPointLight(pointLights[i], norm, fs_in.FragPosition, viewDir, true, perspectiveShadow);
	//}

	// Flashlight (spot light)
	result += CalcSpotLight(norm, fs_in.FragPosition, viewDir, perspectiveShadow);

	// Gamma correction
	float gamma = 2.2;
	result = pow(result, vec3(1.0 / gamma));

	// Set fragment colour to combined result
	FragColour = vec4(result, 1.0);
}

float ShadowCalculationOrthographic(DirLight dirLight, vec4 fragPosLightSpace, vec3 normal)
{
	// Perform perspective division
	// (not neccesary for orthographic projection)
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Depth map is in the range [0,1] and projCoords is in NDC which is [-1,1]
	projCoords = projCoords * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMapOrthographic, projCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	// Check whether current fragment is in shadow
	float bias = max(0.05 * (1.0 - dot(normal, dirLight.direction)), 0.005);
	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	// PCF (Percentage-closer filtering)
	// Takes average of nearest depth buffer locations for smoother shadow edges
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMapOrthographic, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMapOrthographic, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0)
		shadow = 0.0;
	return shadow;
}

float ShadowCalculationPerspective(vec4 fragPosLightSpace, vec3 normal)
{
	// Perform perspective division
	// (not neccesary for orthographic projection)
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Depth map is in the range [0,1] and projCoords is in NDC which is [-1,1]
	projCoords = projCoords * 0.5 + 0.5;
	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMapPerspective, projCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	// Check whether current fragment is in shadow
	float bias = 0.0; // for perspective projection
	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	// PCF (Percentage-closer filtering)
	// Takes average of nearest depth buffer locations for smoother shadow edges
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMapPerspective, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMapPerspective, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0)
		shadow = 0.0;
	return shadow;
}

vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir, float shadow)
{
	// Ambient
	vec3 ambient = dirLight.ambient * vec3(texture(u_Material.diffuse, fs_in.TexCoords));
	//
	// Diffuse 
	vec3 lightDir = dirLight.direction;
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = dirLight.diffuse * diff * vec3(texture(u_Material.diffuse, fs_in.TexCoords));
	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	vec3 specular = (spec * u_Material.specular) * dirLight.specular;

	// Combine 
	return (ambient + (diffuse * shadow) + (specular * shadow));
}

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, bool blinnPhongEnabled, float shadow)
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
	return (ambient + (diffuse * shadow) + (specular * shadow));

}

vec3 CalcSpotLight(vec3 norm, vec3 FragPos, vec3 viewDir, float shadow)
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
		fl_ambient = u_Flashlight.ambient * vec3(texture(u_Material.diffuse, fs_in.TexCoords));
		// flashlight diffuse 
		float fl_diff = max(0.0, dot(norm, flashlightDir));
		fl_diffuse = u_Flashlight.diffuse * fl_diff * vec3(texture(u_Material.diffuse, fs_in.TexCoords));
		// flashlight specular 
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
	return (fl_ambient + (fl_diffuse * shadow) + (fl_specular * shadow));
}
