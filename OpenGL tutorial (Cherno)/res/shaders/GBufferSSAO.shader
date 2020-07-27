#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TextureCoords;

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

	// Want to pass FragPosition in view space since SSAO is a screen-space algorithm
	FragPosition = (view * model * vec4(a_Position, 1.0)).xyz;
	TexCoords = a_TextureCoords;
	gl_Position = proj * view * model * vec4(a_Position, 1.0);
}


#shader fragment

#version 330 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in vec3 FragPosition;
in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;

void main()
{
	// Store the fragment position vector in the first gbuffer texture
	gPosition = FragPosition;
	// Store the per-fragment normals into the second gbuffer texture
	gNormal = normalize(Normal);
	// Store the diffuse per-fragment colour into the rgb components of the third gbuffer texture
	gAlbedoSpec.rgb = texture(texture_diffuse0, TexCoords).rgb;
	//gAlbedoSpec.rgb = vec3(0.95);
	// Store specular intensity in gAlbedoSpec's alpha component
	gAlbedoSpec.a = texture(texture_specular0, TexCoords).r;

	// Testing
	//gPosition =			texture(texture_diffuse0, TexCoords).rgb;
	////gPosition =			vec3(1.0, 0.0, 0.0);
	//gNormal =			texture(texture_diffuse0, TexCoords).rgb;
	////gAlbedoSpec.rgb =	texture(texture_diffuse0, TexCoords).rgb;
	//gAlbedoSpec.rgb =	vec3(1.0, 0.0, 0.0);
	//gAlbedoSpec.a =		texture(texture_specular0, TexCoords).r;
}
