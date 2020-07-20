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
out vec4 FragColour;

in vec2 v_TexCoords;

uniform sampler2D framebufferTexture;

void main()
{
    // Tone mapping is the process of transforming from High Dynamic Range (HDR) [0.0 to any float value] back to Low Dynamic Range (LDR) [0.0, 1.0]
    const float gamma = 2.2;
    vec3 hdrColour = texture(framebufferTexture, v_TexCoords).rgb;
    // Reinhard tone mapping
    vec3 ldrColour = hdrColour / (hdrColour + vec3(1.0));
    // Gamma correction already done in other shader
    // ldrColour = pow(ldrColour, vec3(1.0 / gamma));

    FragColour = vec4(ldrColour, 1.0);
}