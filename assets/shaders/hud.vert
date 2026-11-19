#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=2) in vec2 aUv;
uniform mat4 uProjection;
uniform vec3 uOrigin, uSize;
uniform int uGlyphs[16];
flat out int glyph;
out vec2 uv;
void main() {
    uv = aUv;
    glyph = uGlyphs[gl_InstanceID];
    // Moi o atlas rong 8 pixel, buoc chu la 6 pixel de giu khoang cach cua font 5x7.
    vec2 origin = uOrigin.xy + vec2(float(gl_InstanceID) * (6.0 / 8.0) * uSize.x, 0.0);
    gl_Position = uProjection * vec4(origin + (aPosition.xy + 0.5) * uSize.xy, 0, 1);
}
