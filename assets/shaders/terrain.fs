#version 330

in vec3 fragPos;
flat in vec3 fragNormal;  // <- FLAT anche qui!

out vec4 finalColor;

uniform vec3 lightDir = vec3(-0.5, -1.0, -0.5);

void main()
{
    vec3 N = fragNormal;  // Già normalizzata, NON interpolata
    vec3 L = normalize(-lightDir);
    
    float diff = max(dot(N, L), 0.0);
    
    // Colori in base all'altezza
    vec3 baseColor;
    if (fragPos.y < 4.0) {
        baseColor = vec3(0.2, 0.15, 0.1);
    } else if (fragPos.y < 8.0) {
        baseColor = vec3(0.4, 0.28, 0.18);
    } else {
        baseColor = vec3(0.6, 0.45, 0.3);
    }
    
    vec3 ambient = vec3(0.3);
    vec3 lighting = ambient + vec3(1.0, 0.95, 0.85) * diff * 1.5;
    
    finalColor = vec4(baseColor * lighting, 1.0);
}