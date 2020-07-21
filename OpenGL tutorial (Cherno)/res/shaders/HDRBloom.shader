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

uniform sampler2D hdrImageTexture;
uniform sampler2D bloomImageTexture;
uniform bool u_UsingHDR;
uniform float u_Exposure;

void main()
{
    if (u_UsingHDR) {
        // Tone mapping is the process of transforming from High Dynamic Range (HDR) [0.0 to any float value] back to Low Dynamic Range (LDR) [0.0, 1.0]
        vec3 hdrColour = texture(hdrImageTexture, v_TexCoords).rgb;
        vec3 bloomColour = texture(bloomImageTexture, v_TexCoords).rgb;
        hdrColour += bloomColour; // additive blending of HDR and Bloom colour buffers
        // Exposure tone mapping
        vec3 ldrColour = vec3(1.0) - exp(-hdrColour * u_Exposure);

        FragColour = vec4(ldrColour, 1.0);
    }
    else {
        // No tone mapping (just upper bounds at 1.0)
        vec3 ldrColour = texture(hdrImageTexture, v_TexCoords).rgb;

        FragColour = vec4(ldrColour, 1.0);
    }
    
}