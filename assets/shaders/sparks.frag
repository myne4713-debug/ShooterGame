#version 330 core
in vec2 uv;
in float heat;
out vec4 fragColor;
void main() {
    // Mặt nạ elip trong hệ UV làm mềm mép vệt sáng ngay trên quad thủ tục.
    float mask = 1.0 - smoothstep(0.25, 1.0, length(2.0 * uv - 1.0));
    fragColor = vec4(1.0, 0.35 + 0.6 * heat, 0.12 + 0.45 * heat, heat * mask);
}
