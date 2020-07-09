#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TextureCoords;

out vec2 TexCoords;
//out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {

	// TODO: should pass this as a uniform to optimize (costly to perform matrix inverse in shaders)
	//mat3 normalMatrix = mat3(transpose(inverse(model)));
	//Normal = normalMatrix * a_Normal;
	
	TexCoords = a_TextureCoords;
    gl_Position = proj * view * model * vec4(a_Position, 1.0);
}


#shader fragment
#version 330 core

in vec2 TexCoords;

out vec4 FragColour;

uniform sampler2D texture_diffuse0;

void main() {
	FragColour = texture(texture_diffuse0, TexCoords);
}