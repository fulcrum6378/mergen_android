#version 450

// input data from C++
layout(binding = 0) uniform UniformBufferObject {
    mat4 mvp;
} ubo;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 colour;

// passed to the fragment shader
layout(location = 0) out vec3 fragColor;

// invoked on every vertex (3 times) changing gl_VertexIndex
void main() {
    gl_Position = ubo.mvp * vec4(pos, /*z:*/0.0, /*w:*/1.0);
    fragColor = colour;
}
