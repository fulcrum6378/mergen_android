#version 450

// The input variable does not necessarily have to use the same name.
// Input colour coming from the vertex shader
layout(location/*of framebuffer*/ = 0) in/*put*/ vec3 fragColor;

// Output colour for the fragment
layout(location/*of framebuffer*/ = 0) out/*put*/ vec4 outColor;

/** Invoked on every fragment of the one and only framebuffer */
void main() {
    outColor = vec4(fragColor, /*alpha:*/1.0);
}
