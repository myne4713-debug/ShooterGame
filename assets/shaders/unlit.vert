#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uColor;
out vec3 faceColor;
out vec2 uv;
out vec3 eyePosition;

void main() {
    vec4 eye = uView * uModel * vec4(aPosition, 1.0);
    gl_Position = uProjection * eye;
    eyePosition = eye.xyz;
    uv = aUv;
    // Moi cap mat truc X, Y, Z co sac do rieng de de thay hinh khoi.
    faceColor = uColor * dot(abs(aNormal), vec3(0.72, 1.0, 0.87));
}
