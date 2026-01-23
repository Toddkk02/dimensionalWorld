#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // Transform position to world space
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    
    // Transform normal
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    
    // Pass through
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Final position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}