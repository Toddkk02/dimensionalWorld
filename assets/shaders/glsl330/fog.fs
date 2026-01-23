#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform float fogDensity;
uniform vec4 fogColor;

out vec4 finalColor;

void main()
{
    // Sample texture
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 baseColor = texelColor.rgb * fragColor.rgb * colDiffuse.rgb;
    
    // Simple lighting
    vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.5));
    vec3 normal = normalize(fragNormal);
    float diff = max(dot(normal, -lightDir), 0.0);
    vec3 ambient = vec3(0.3);
    vec3 lighting = ambient + vec3(0.7) * diff;
    
    vec3 litColor = baseColor * lighting;
    
    // Fog calculation - LINEAR FOG
    float distance = length(viewPos - fragPosition);
    float fogStart = 10.0;
    float fogEnd = 50.0;
    float fogFactor = (fogEnd - distance) / (fogEnd - fogStart);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    // Mix fog
    vec3 finalRGB = mix(fogColor.rgb, litColor, fogFactor);
    
    finalColor = vec4(finalRGB, texelColor.a);
}