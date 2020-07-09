#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TextureCoords;

out VS_OUT{
    vec2 texCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {

    vs_out.texCoords = a_TextureCoords;

    gl_Position = proj * view * model * vec4(a_Position, 1.0);
}


#shader geometry
#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT{
    vec2 texCoords;
} gs_in[];

out vec2 TexCoords;

uniform float u_time;

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 4.0;
    vec3 direction = normal * ((sin(u_time/6.0f) + 1.0) / 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

void main() {
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].texCoords;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[1].texCoords;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    EmitVertex();
    EndPrimitive();
}


#shader fragment
#version 330 core

in vec2 TexCoords;

out vec4 FragColour;

uniform sampler2D texture_diffuse0;

void main() {
    FragColour = texture(texture_diffuse0, TexCoords);
}