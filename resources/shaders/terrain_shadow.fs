#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec4 fragColor;
in vec2 fragTexCoord;
in vec4 fragPositionLightSpace;

// Input uniform values
uniform sampler2D shadowMap;
uniform vec3 lightDir;
uniform float shadowIntensity;
uniform float shadowBias;
uniform int shadowSamples; // PCF sampling amount (e.g. 1 means -1 to 1 = 3x3)

// Output fragment color
out vec4 finalColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check whether current frag pos is in shadow
    float currentDepth = projCoords.z;
    
    // PCF (Percentage Closer Filtering) for smoother shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    // If shadowSamples is 0, we just do a single point sample
    if (shadowSamples <= 0) {
        float depth = texture(shadowMap, projCoords.xy).r;
        shadow = currentDepth - shadowBias > depth ? 1.0 : 0.0;
    } else {
        float count = 0.0;
        for(int x = -shadowSamples; x <= shadowSamples; ++x)
        {
            for(int y = -shadowSamples; y <= shadowSamples; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += (currentDepth - shadowBias > pcfDepth) ? 1.0 : 0.0;
                count += 1.0;
            }    
        }
        shadow /= count;
    }

    // Keep shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
    float shadow = ShadowCalculation(fragPositionLightSpace);
    
    // Darken the color in shadow
    vec3 shadowedColor = fragColor.rgb * (1.0 - shadow * shadowIntensity);
    
    finalColor = vec4(shadowedColor, fragColor.a);
}
