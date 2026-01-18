#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

out vec3 fragPos;
flat out vec3 fragNormal;  // <- FLAT qui!

void main()
{
    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));
    fragNormal = mat3(transpose(inverse(matModel))) * vertexNormal;
    gl_Position = matProjection * matView * vec4(fragPos, 1.0);
}