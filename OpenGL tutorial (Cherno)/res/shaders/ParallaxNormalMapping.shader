#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;
}

#shader fragment
#version 330 core
out vec4 FragColour;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float height_scale;
uniform bool u_UsingParallaxMapping;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main()
{
    // Offset texture coordinates using Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords;
    if(u_UsingParallaxMapping)
        texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
    else
        texCoords = fs_in.TexCoords;
    if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;
    //vec2 texCoords = fs_in.TexCoords;

    // Use the new texture coords to sample texture
    vec3 diffuse = texture(diffuseMap, texCoords).rgb;

    // Get normal from normal map then scale within range [-1.0, 1.0]
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    //FragColour = vec4(diffuse, 1.0);

    float lightAmountTest = dot(normalize(vec3(1.0,1.0,1.0)), normal);
    FragColour = vec4(lightAmountTest * diffuse, 1.0);
    //FragColour = vec4(normal, 1.0);
    //FragColour = texture(depthMap, texCoords);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) 
{
    // Regular Parallax Mapping
    //float height = texture(depthMap, texCoords).r;
    //vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    //// Parallax Mapping with Offset Limiting doesn't use the division by the z component
    //// vec2 p = viewDir.xy * (height * height_scale);
    //return texCoords - p;

    // Steep Parallax Mapping
    // number of depth layers increases with viewing angle
    const float minLayers = 16.0;
    const float maxLayers = 64.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    //return currentTexCoords; // for basic Steep Parallax Mapping

    // Parallax Occlusion Mapping (linearly interpolated version of Steep Parallax Mapping)
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}
