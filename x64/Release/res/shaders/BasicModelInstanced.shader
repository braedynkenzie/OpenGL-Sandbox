#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
//layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TextureCoords;
layout(location = 3) in mat4 instanceModelMatrix;

out vec2 TexCoords;

// uniform mat4 model; // Replaced with instanceModelMatrix
uniform mat4 view;
uniform mat4 proj;

void main() {
	TexCoords = a_TextureCoords;
	gl_Position = proj * view * instanceModelMatrix * vec4(a_Position, 1.0);
}


#shader fragment
#version 330 core

in vec2 TexCoords;

out vec4 FragColour;

uniform sampler2D texture_diffuse0;

void main() {
	vec3 dirLight = vec3(-1.0, -1.0, -1.0);
	vec3 result = vec3(0.0);

	// Ambient light
	vec3 ambient = vec3(texture(texture_diffuse0, TexCoords)) / 1.0;
	result += ambient;
	FragColour = vec4(result, 1.0);
}