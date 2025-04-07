#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 color;

uniform sampler2D tex0;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.0, 10.0, 0.0) - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec4 texColor = texture(tex0, TexCoord);
    color = vec4(diff * texColor.rgb, texColor.a);
}
