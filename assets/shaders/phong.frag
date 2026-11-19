#version 330 core
struct Material {
    vec3 color;
    float shininess;
};
struct PointLight {
    vec3 position;
    vec3 color;
};
uniform Material uMaterial;
uniform sampler2D uAlbedo;
uniform sampler2DShadow uShadowMap;
uniform bool uShadows;
in vec4 lightClip;
uniform vec3 uEye, uFront;
uniform vec3 uAmbient, uSunDirection, uSunColor;
uniform PointLight uPoints[4];
uniform int uPointCount;
uniform vec3 uAttenuation;
uniform bool uSpotOn;
uniform vec3 uSpotColor;
uniform float uInnerCos, uOuterCos;
uniform float uFogStart, uFogEnd;
uniform vec3 uFogColor;
in vec3 worldPosition, worldNormal;
in vec2 uv;
out vec4 fragColor;

vec3 blinnPhong(vec3 N, vec3 V, vec3 L, vec3 albedo) {
    float diffuse = max(dot(N, L), 0.0);
    if (diffuse <= 0.0) return vec3(0.0);
    // H nam giua huong toi mat va toi den. Shininess lon thu hep vet bong.
    vec3 H = normalize(V + L);
    float specular = pow(max(dot(N, H), 0.0), uMaterial.shininess);
    return albedo * diffuse + vec3(0.26 * specular);
}

float attenuation(float distanceToLight) {
    return 1.0 / dot(uAttenuation, vec3(1.0, distanceToLight, distanceToLight * distanceToLight));
}

void main() {
    vec3 albedo = texture(uAlbedo, uv).rgb * uMaterial.color;
    vec3 N = normalize(worldNormal);
    vec3 eyeDelta = uEye - worldPosition;
    float eyeDistance = length(eyeDelta);
    vec3 V = normalize(eyeDelta);
    vec3 color = albedo * uAmbient;
    vec3 Lsun = normalize(-uSunDirection);
    float visibility = 1.0;
    if (uShadows) {
        // Đưa tọa độ clip của đèn về [0,1] để UV và độ sâu cùng thang với shadow map.
        vec3 p = (lightClip.xyz / lightClip.w) * 0.5 + 0.5;
        if (all(greaterThanEqual(p, vec3(0.0))) && all(lessThanEqual(p, vec3(1.0)))) {
            visibility = texture(uShadowMap, p);
        }
    }
    color += visibility * uSunColor * blinnPhong(N, V, Lsun, albedo);
    for (int i = 0; i < uPointCount; ++i) {
        vec3 delta = uPoints[i].position - worldPosition;
        float d = length(delta);
        vec3 L = delta / max(d, 0.0001);
        color += uPoints[i].color * attenuation(d) * blinnPhong(N, V, L, albedo);
    }
    if (uSpotOn) {
        // Den pin o mat nen -V huong tu den toi fragment. Hai goc cat tao vien non mem.
        float cone = smoothstep(uOuterCos, uInnerCos, dot(uFront, -V));
        color += uSpotColor * cone * attenuation(eyeDistance) * blinnPhong(N, V, V, albedo);
    }
    float fog = smoothstep(uFogStart, uFogEnd, eyeDistance);
    fragColor = vec4(mix(color, uFogColor, fog), 1.0);
}
