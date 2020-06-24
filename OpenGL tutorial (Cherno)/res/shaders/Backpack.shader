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

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;

struct Material {
	vec3 specular;
	float shininess;
};
uniform Material u_Material;

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

// Function declarations
vec3 CalcSpotLight(vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {

	// Phong lighting (using directional, point lights, spotlights)
	//
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPosition);
	vec3 result = vec3(0.0f);

	// Directional lighting
	//for (int i = 0; i < NUM_DIR_LIGHTS; i++)
	//	result += CalcDirLight(dirLights[i], norm, viewDir);

	// Point lights
	//for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	//	result += CalcPointLight(pointLights[i], norm, FragPosition, viewDir);

	// Spot light (flashlight)
	result += CalcSpotLight(norm, FragPosition, viewDir);

	// Combined result
	FragColour = vec4(result, 1.0);
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
		fl_ambient = u_Flashlight.ambient * vec3(texture(texture_diffuse0, TexCoords));
		// flashlight diffuse 
		float fl_diff = max(0.0, dot(norm, flashlightDir));
		fl_diffuse = u_Flashlight.diffuse * fl_diff * vec3(texture(texture_diffuse0, TexCoords));
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
