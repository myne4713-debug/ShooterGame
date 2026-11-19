#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;

uniform mat4 uModel, uView, uProjection;
uniform mat4 uLightVP;
out vec4 lightClip;
uniform vec3 uTiles;
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 uv;

void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    worldPosition = world.xyz;
    lightClip = uLightVP * world;
    // Co gian khong deu lam doi goc. Nghich dao chuyen vi giu phap tuyen vuong goc mat.
    worldNormal = transpose(inverse(mat3(uModel))) * aNormal;
    // UV cua primitive chay theo Y tren mat dung va Z tren mat nam.
    float tilesU = abs(aNormal.x) > 0.5 ? uTiles.z : uTiles.x;
    float tilesV = abs(aNormal.y) > 0.5 ? uTiles.z : uTiles.y;
    uv = aUv * vec2(tilesU, tilesV);
    gl_Position = uProjection * uView * world;
}
