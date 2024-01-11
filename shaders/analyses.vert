#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 colour;

layout(location = 0) out vec3 fragColor;

// invoked on every vertex changing gl_VertexIndex
void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, /*z:*/0.0, /*w:*/1.0);
    fragColor = colour;
}
