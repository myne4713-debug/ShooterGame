#version 330 core
in vec3 faceColor;
in vec2 uv;
in vec3 eyePosition;
uniform bool uFloor;
uniform vec3 uSky;
uniform float uFogStart;
uniform float uFogEnd;
out vec4 fragColor;

void main() {
    vec3 color = faceColor;
    if (uFloor) {
        float tile = mod(floor(uv.x) + floor(uv.y), 2.0);
        color *= mix(0.88, 1.0, tile);
    }
    // Goc nhin co goc toa do tai mat, nen do dai vector chinh la khoang cach toi mat.
    float fog = smoothstep(uFogStart, uFogEnd, length(eyePosition));
    fragColor = vec4(mix(color, uSky, fog), 1.0);
}
