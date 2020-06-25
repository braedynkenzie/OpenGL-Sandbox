#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TextureCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
	gl_Position = proj * view * model * vec4(a_Position, 1.0);
}


#shader fragment
#version 330 core
out vec4 FragColor;

uniform vec3 pointLightColour;

void main() {
	FragColor = vec4(pointLightColour, 1.0);
}