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

//void main()
//{
//    FragColour = texture(framebufferTexture, v_TexCoords);
//}

const float offset = 1.0 / 300.0;

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset), // top-left
        vec2(0.0f, offset), // top-center
        vec2(offset, offset), // top-right
        vec2(-offset, 0.0f),   // center-left
        vec2(0.0f, 0.0f),   // center-center
        vec2(offset, 0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f, -offset), // bottom-center
        vec2(offset, -offset)  // bottom-right    
        );

    // Sharpen edges kernel
    /*float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
        );*/

    // Blur effect kernel
    int sum = 24;
    float kernel[9] = float[](
        4.0 / sum, 2.0 / sum, 4.0 / sum,
        2.0 / sum, 0.0 / sum, 2.0 / sum,
        4.0 / sum, 2.0 / sum, 4.0 / sum
        );

    // Line detection kernel
    /*int sum = 1;
    float kernel[9] = float[](
        1.0 / sum,  1.0 / sum, 1.0 / sum,
        1.0 / sum, -8.0 / sum, 1.0 / sum,
        1.0 / sum,  1.0 / sum, 1.0 / sum
        );*/

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(framebufferTexture, v_TexCoords.st + offsets[i]));
    }
    vec3 colour = vec3(0.0);
    for (int i = 0; i < 9; i++)
        colour += sampleTex[i] * kernel[i];

    FragColour = vec4(colour, 1.0);
}