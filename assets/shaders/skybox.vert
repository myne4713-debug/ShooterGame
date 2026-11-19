#version 330 core
layout (location = 0) in vec3 aPosition;
uniform mat4 uView, uProjection;
out vec3 direction;
void main() {
    direction = aPosition;
    // Bo tinh tien de bau troi chi xoay theo mat. z = w day do sau ve mat phang xa.
    vec4 clip = uProjection * mat4(mat3(uView)) * vec4(aPosition, 1.0);
    gl_Position = clip.xyww;
}
