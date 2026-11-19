#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aUv;
uniform mat4 uView, uProjection;
uniform vec3 uOrigin, uNormal;
uniform float uAge, uLifetime;
out vec2 uv;
out float heat;

void main() {
    // Cùng một quad, mỗi instance chọn một hướng trong bán cầu phía ngoài mặt trúng.
    float id = float(gl_InstanceID);
    const float goldenAngle = 2.399963;
    float angle = id * goldenAngle;
    vec3 ref = abs(uNormal.y) < 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0);
    vec3 tangent = normalize(cross(ref, uNormal));
    vec3 ring = cos(angle) * tangent + sin(angle) * cross(uNormal, tangent);
    vec3 velocity = (1.2 + fract(id * 0.37)) * uNormal + (0.8 + 1.8 * fract(id * 0.71)) * ring;
    const vec3 gravity = vec3(0, -5, 0);
    vec3 center = uOrigin + velocity * uAge + 0.5 * gravity * uAge * uAge;
    vec4 eye = uView * vec4(center, 1.0);
    // Kéo dài quad theo hình chiếu vận tốc trong hệ mắt để vệt sáng luôn nhìn thấy được.
    vec2 speed = (mat3(uView) * (velocity + gravity * uAge)).xy;
    vec2 axis = length(speed) > 0.001 ? normalize(speed) : vec2(0, 1);
    vec2 side = vec2(axis.y, -axis.x);
    eye.xy += side * aPosition.x * 0.035 + axis * aPosition.y * (0.06 + 0.04 * length(speed));
    gl_Position = uProjection * eye;
    uv = aUv;
    heat = max(0.0, 1.0 - uAge / (uLifetime * (0.7 + 0.3 * fract(id * 0.53))));
}
