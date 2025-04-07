#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {
    FragPos = vec3(M * vec4(vertex, 1.0));
    Normal = mat3(transpose(inverse(M))) * normal;
    TexCoord = texCoord0;

    gl_Position = P * V * vec4(FragPos, 1.0);
}
