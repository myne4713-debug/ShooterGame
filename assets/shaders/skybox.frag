#version 330 core
in vec3 direction;
uniform samplerCube uSky;
uniform float uBrightness;
out vec4 fragColor;
void main() {
    fragColor = vec4(uBrightness * texture(uSky, direction).rgb, 1.0);
}
