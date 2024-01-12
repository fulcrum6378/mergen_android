#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    // put colours here
} ubo;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec3 fragColor;

// invoked on every vertex changing gl_VertexIndex
void main() {
    gl_Position = vec4(pos, /*z:*/0.0, /*w:*/1.0);
    fragColor = vec3(1.0, 0.0, 0.0);
}
