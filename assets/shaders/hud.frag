#version 330 core
in vec2 uv;
flat in int glyph;
uniform sampler2D uFont;
uniform vec3 uColor;
uniform float uAlpha;
out vec4 fragColor;
void main() {
    float mask = 1.0;
    if (glyph >= 0) {
        // Moi ma ASCII chiem mot o vuong trong atlas mot hang.
        mask = texture(uFont, vec2((float(glyph) + uv.x) / 128.0, uv.y)).r;
    }
    fragColor = vec4(uColor, uAlpha * mask);
}
