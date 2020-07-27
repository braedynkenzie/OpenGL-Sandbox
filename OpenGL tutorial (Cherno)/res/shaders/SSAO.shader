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
out float FragColour;
//out vec3 FragColour; // debugging

in vec2 v_TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform int u_Screen_Width;
uniform int u_Screen_Height;
uniform mat4 projMatrix;

// Debugging purposes only
uniform sampler2D gAlbedoSpec;

uniform vec3 samples[64];

// SSAO parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 1.0; // 0.5;
float bias = 0.025;

void main()
{
    // Get input data for SSAO algorithm from filled GBuffer textures
    vec3 fragPos = texture(gPosition, v_TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, v_TexCoords).rgb);
    // tile noise texture over screen based on screen dimensions
    vec2 noiseScale = vec2(u_Screen_Width / 4.0, u_Screen_Height / 4.0);
    vec3 randomVec = normalize(texture(texNoise, v_TexCoords * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // from tangent to view-space
        sample = fragPos + sample * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = projMatrix * offset;        // from view to clip-space
        offset.xyz /= offset.w;              // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample

        // range check
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        // accumulate number of points that are occluded
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);

    // FragColour is a single float for this framebuffer's colour attachment
    FragColour = occlusion;
    


    // Testing
    //vec3 diffuse = texture(gAlbedoSpec, v_TexCoords).xyz; // Debugging purposes only
    //FragColour = diffuse;
}
