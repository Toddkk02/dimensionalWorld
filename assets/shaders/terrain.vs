#version 330

// Input dai vertici
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;  // coordinate UV

// Matrici uniform
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

// Output verso il fragment shader
out vec3 fragPos;
flat out vec3 fragNormal;
out vec2 fragUV;

void main()
{
    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));
    fragNormal = mat3(transpose(inverse(matModel))) * vertexNormal;
    fragUV = vertexTexCoord;
    gl_Position = matProjection * matView * vec4(fragPos, 1.0);
}
