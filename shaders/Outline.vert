#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float outlineThickness;

void main() {
    vec4 pos = model * vec4(aPos, 1.0);
    vec3 dir = normalize(pos.xyz);
    pos.xyz += dir * outlineThickness;
    gl_Position = projection * view * pos;
}