#version 330 core

in vec2 vUV;
in vec3 vWorldPos;
out vec4 fragColor;

uniform sampler2D treasureTex;
uniform bool uFogEnabled;
uniform int uFogType;
uniform float uFogDensity;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uFogColor;
uniform vec3 uFogCameraPos;

void main() {
    vec4 color = texture(treasureTex, vUV);
    if (color.a < 0.05) {
        discard;
    }
    if (uFogEnabled) {
        float distance = length(uFogCameraPos - vWorldPos);
        float fogFactor;
        if (uFogType == 0) {
            fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);
        } else if (uFogType == 1) {
            fogFactor = exp(-uFogDensity * distance);
        } else {
            float d = uFogDensity * distance;
            fogFactor = exp(-d * d);
        }

        float heightFog = exp(-max(vWorldPos.y - 0.2, 0.0) * 0.4);
        fogFactor = clamp(fogFactor * heightFog, 0.0, 1.0);
        vec3 finalColor = mix(uFogColor, color.rgb, fogFactor);
        fragColor = vec4(finalColor, color.a);
    } else {
        fragColor = color;
    }
}
