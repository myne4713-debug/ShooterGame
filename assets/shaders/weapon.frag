#version 330 core
in vec2 uv;
in vec3 normal;
uniform vec3 uColor;
uniform int uMode; // 0: mesh, 1: decal, 2: tracer, 3: flash
uniform float uAlpha;
out vec4 fragColor;
void main() {
    vec3 color = uColor;
    float alpha = uAlpha;
    if (uMode == 0) {
        color *= 0.55 + 0.45 * max(dot(normalize(normal), normalize(vec3(-0.4, 0.7, 0.5))), 0.0);
    } else if (uMode == 1 || uMode == 3) {
        vec2 p = 2.0 * uv - 1.0;
        float r = length(p);
        if (uMode == 3) {
            r *= 1.0 + 0.30 * cos(6.0 * atan(p.y, p.x));
            color = mix(vec3(1.0, 0.96, 0.72), color, smoothstep(0.0, 0.6, r));
        }
        alpha *= 1.0 - smoothstep(0.45, 1.0, r);
    }
    fragColor = vec4(color, alpha);
}
