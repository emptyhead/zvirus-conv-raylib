#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec4 vertexColor;
in vec2 vertexTexCoord;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matLightVP; // Light Projection * Light View

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec4 fragColor;
out vec2 fragTexCoord;
out vec4 fragPositionLightSpace;

void main()
{
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragColor = vertexColor;
    fragTexCoord = vertexTexCoord;
    
    // Project position into light space
    fragPositionLightSpace = matLightVP * vec4(fragPosition, 1.0);
    
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
