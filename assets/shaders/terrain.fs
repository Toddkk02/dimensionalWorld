#version 330

in vec3 fragPos;
flat in vec3 fragNormal;
in vec2 fragUV;

out vec4 finalColor;

// Texture uniforms
uniform sampler2D textureTop;
uniform sampler2D textureSide;
uniform sampler2D textureDirt;

// Luce
uniform vec3 lightDir = vec3(-0.5, -1.0, -0.5);

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(-lightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 ambient = vec3(0.3);
    vec3 lighting = ambient + vec3(1.0, 0.95, 0.85) * diff;

    // Scegli la texture in base alla normale
    vec3 baseColor;
    if (abs(N.y) > 0.9) // superficie superiore/inferiore
    {
        baseColor = texture(textureTop, fragUV).rgb;
    }
    else if (abs(N.x) > 0.5 || abs(N.z) > 0.5) // lati
    {
        baseColor = texture(textureSide, fragUV).rgb;
    }
    else // fallback
    {
        baseColor = texture(textureDirt, fragUV).rgb;
    }

    finalColor = vec4(baseColor * lighting, 1.0);
}
