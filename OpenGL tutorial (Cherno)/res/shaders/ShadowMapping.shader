#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix; // proj * view matrix
uniform mat4 lightModel;

void main()
{
	gl_Position = lightSpaceMatrix * lightModel * vec4(aPos, 1.0);
}


#shader fragment
#version 330 core

void main()
{
	// Explicitly set the fragment depth in the shadow map
	// (Default would do exactly this in the background anyway)
	gl_FragDepth = gl_FragCoord.z;
}
