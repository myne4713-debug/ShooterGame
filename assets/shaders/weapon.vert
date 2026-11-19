#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUv;
uniform mat4 uVP, uModel;
out vec2 uv;
out vec3 normal;
void main() {
    uv = aUv;
    normal = transpose(inverse(mat3(uModel))) * aNormal;
    gl_Position = uVP * uModel * vec4(aPosition, 1.0);
}
