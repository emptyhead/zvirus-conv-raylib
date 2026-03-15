#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

void main()
{
    float depth = texture(texture0, fragTexCoord).r;
    // Linearize depth if it's too squashed (optional, depends on camera)
    // For now just show it directly.
    finalColor = vec4(vec3(depth), 1.0);
}
