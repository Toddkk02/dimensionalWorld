#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float aberration;

out vec4 finalColor;

void main()
{
    vec2 uv = fragTexCoord;
    
    // Distance from center
    vec2 centerOffset = uv - vec2(0.5);
    float dist = length(centerOffset);
    
    // Direction from center
    vec2 direction = centerOffset / (dist + 0.001);
    
    // Sample RGB channels with offset
    float r = texture(texture0, uv + direction * aberration * dist).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - direction * aberration * dist).b;
    
    finalColor = vec4(r, g, b, 1.0);
}