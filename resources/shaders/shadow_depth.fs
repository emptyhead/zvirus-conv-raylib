#version 330

// Output fragment color
out vec4 finalColor;

void main()
{
    // Depth is handled automatically by the depth buffer
    // But we need a fragment shader to be a valid program
    // gl_FragCoord.z contains the depth value
}
