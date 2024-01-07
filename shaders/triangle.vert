#version 450

// Colour passed to the fragment shader
layout(location = 0) out/*put*/ vec3 fragColor;

// Uniform buffer containing an MVP matrix.
// Currently the vulkan backend only sets the rotation matix
// required to handle device rotation.
layout(binding = 0) uniform UniformBufferObject {
    mat4 MVP;
} ubo;

/*vec2 positions[3] = vec2[](
vec2(0.0, -0.5), // top
vec2(0.5, 0.5), // right
vec2(-0.5, 0.5)// left
);

vec3 colors[3] = vec3[](// Madde3e's yellow dress (#EAB044)
vec3(0.9176, 0.6902, 0.2667),
vec3(0.9176, 0.6902, 0.2667),
vec3(0.9176, 0.6902, 0.2667)
);// Her hair: #332D39*/

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

/** Invoked on every vertex (3 times) changing gl_VertexIndex */
void main() {
    gl_Position = ubo.MVP * vec4(inPosition, /*z:*/0.0, /*w:*/1.0);
    fragColor = inColor;
}
