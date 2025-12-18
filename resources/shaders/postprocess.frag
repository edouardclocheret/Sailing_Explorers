#version 330 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uColorTex;
uniform sampler2D uDepthTex;
uniform int uMode; // 0 = color, 1 = depth, 2 = DOF
uniform float uNear;
uniform float uFar;

// DOF uniforms
uniform bool uDofEnabled;
uniform float uFocalDistance;
uniform float uAperture;
uniform vec2 uTexelSize; // 1.0 / textureSize

float linearizeDepth(float depthSample) {
    float z = depthSample * 2.0 - 1.0;
    return (2.0 * uNear * uFar) / max(uFar + uNear - z * (uFar - uNear), 1e-6);
}

float computeCoC(float linearDepth) {
    float depthDiff = abs(linearDepth - uFocalDistance);
    float coc = uAperture * depthDiff / max(linearDepth, 0.001);
    return clamp(coc * 100.0, 0.0, 20.0);
}

const int SAMPLE_COUNT = 16;
const vec2 poissonDisk[16] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2( 0.94558609, -0.76890725),
    vec2(-0.09418410, -0.92938870),
    vec2( 0.34495938,  0.29387760),
    vec2(-0.91588581,  0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543,  0.27676845),
    vec2( 0.97484398,  0.75648379),
    vec2( 0.44323325, -0.97511554),
    vec2( 0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2( 0.79197514,  0.19090188),
    vec2(-0.24188840,  0.99706507),
    vec2(-0.81409955,  0.91437590),
    vec2( 0.19984126,  0.78641367),
    vec2( 0.14383161, -0.14100790)
);

vec4 dofBlur() {
    float centerDepth = texture(uDepthTex, vUV).r;
    float centerLinearDepth = linearizeDepth(centerDepth);
    float centerCoC = computeCoC(centerLinearDepth);
    if (centerCoC < 0.5) {
        return texture(uColorTex, vUV);
    }
    vec4 colorSum = texture(uColorTex, vUV);
    float weightSum = 1.0;
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        vec2 offset = poissonDisk[i] * centerCoC * uTexelSize;
        vec2 sampleUV = clamp(vUV + offset, 0.0, 1.0);
        vec4 sampleColor = texture(uColorTex, sampleUV);
        float sampleDepth = texture(uDepthTex, sampleUV).r;
        float sampleLinearDepth = linearizeDepth(sampleDepth);
        float sampleCoC = computeCoC(sampleLinearDepth);
        float weight = 1.0;
        if (sampleLinearDepth > centerLinearDepth && sampleCoC < centerCoC * 0.5) {
            weight = 0.3;
        }
        colorSum += sampleColor * weight;
        weightSum += weight;
    }
    return colorSum / weightSum;
}

void main() {
    if (uMode == 1) {
        float depth = texture(uDepthTex, vUV).r;
        float linearDepth = linearizeDepth(depth);
        float normalized = clamp(linearDepth / uFar, 0.0, 1.0);
        FragColor = vec4(vec3(normalized), 1.0);
    } else if (uMode == 2) {
        float depth = texture(uDepthTex, vUV).r;
        float linearDepth = linearizeDepth(depth);
        float coc = computeCoC(linearDepth);
        float normalized = coc / 20.0;
        FragColor = vec4(normalized, 0.0, 1.0 - normalized, 1.0);
    } else if (uDofEnabled) {
        FragColor = dofBlur();
    } else {
        FragColor = texture(uColorTex, vUV);
    }
}
