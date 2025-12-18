#version 330 core

struct Light4GPU {
    int valid;
    int type;
    vec3 color;
    vec3 function;
    vec3 pos;
    vec3 dir;
    float penumbra;
    float angle;
};

uniform Light4GPU lights_8[8];

uniform vec3 cAmbient;
uniform vec3 cDiffuse;
uniform vec3 cSpecular;

uniform float ka;
uniform float kd;
uniform float ks;
uniform vec4 cam_pos;
uniform float n;

uniform float u_time; //for scrolling

in vec3 world_space_pos;
in vec3 world_space_normal;
in vec2 uv_out;

uniform sampler2D texture_sampler;   // normal map
uniform bool uFogEnabled;
uniform int uFogType;
uniform float uFogDensity;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uFogColor;
uniform vec3 uFogCameraPos;

out vec4 fragColor;


float getfalloff(float theta, float inner, float outer){
    float q = (theta - inner) / (outer - inner);
    return -2.0 * q*q*q + 3.0 * q*q;
}

void main() {
    // Scroll speed
    vec2 scroll_speed = vec2(0.05, 0.1); // X and Y speed
    vec2 uv_scrolled = uv_out + scroll_speed * u_time;

    vec3 texNormal = texture(texture_sampler, uv_scrolled).xyz;
    texNormal = normalize(texNormal * 2.0 - 1.0);

    //tangent = (1,0,0), bitangent = (0,0,1) for the floor
    vec3 T = normalize(vec3(1.0, 0.0, 0.0));
    vec3 B = normalize(vec3(0.0, 0.0, 1.0));
    vec3 N = normalize(world_space_normal);

    mat3 TBN = mat3(T, B, N);

    vec3 normal = normalize(TBN * texNormal);


    //phong as regular
    fragColor = vec4(ka * cAmbient, 1.0);

    for (int i = 0; i < 8; i++) {
        if (lights_8[i].valid == 0) continue;

        vec3 to_light;
        float fatt;

        if (lights_8[i].type == 0) {
            to_light = normalize(lights_8[i].pos - world_space_pos);
            float dist = distance(lights_8[i].pos, world_space_pos);
            fatt = min(1.0, 1.0 / (lights_8[i].function.x +
                                   dist * lights_8[i].function.y +
                                   dist*dist * lights_8[i].function.z));
        }
        else if (lights_8[i].type == 1) {
            to_light = normalize(-lights_8[i].dir);
            fatt = 1.0;
        }
        else {
            to_light = normalize(lights_8[i].pos - world_space_pos);
            float dist = distance(lights_8[i].pos, world_space_pos);

            fatt = min(1.0, 1.0 / (lights_8[i].function.x +
                                   dist * lights_8[i].function.y +
                                   dist*dist * lights_8[i].function.z));

            float theta = acos(dot(normalize(-to_light), normalize(lights_8[i].dir)));
            float inner = lights_8[i].angle - lights_8[i].penumbra;
            float outer = lights_8[i].angle;

            if (theta < inner) {
                fatt *= 1.0;
            } else if (theta < outer) {
                fatt *= (1.0 - getfalloff(theta, inner, outer));
            } else {
                fatt = 0.0;
            }
        }

        vec3 reflected = reflect(-to_light, normal);

        // Diffuse
        float ndotl = max(dot(normal, to_light), 0.0);
        fragColor.rgb += fatt * lights_8[i].color * kd * cDiffuse * ndotl;

        // Specular
        vec3 cam_dir = normalize(cam_pos.xyz - world_space_pos);
        float dotprod = max(dot(reflected, cam_dir), 0.0);

        if (dotprod > 0.0 && n > 0.0) {
            fragColor.rgb += fatt * lights_8[i].color * ks * cSpecular * pow(dotprod, n);
        }
    }

    if (uFogEnabled) {
        float distance = length(uFogCameraPos - world_space_pos);
        float fogFactor;
        if (uFogType == 0) {
            fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);
        } else if (uFogType == 1) {
            fogFactor = exp(-uFogDensity * distance);
        } else {
            float d = uFogDensity * distance;
            fogFactor = exp(-d * d);
        }

        float heightFog = exp(-max(world_space_pos.y - 0.2, 0.0) * 0.35);
        fogFactor = clamp(fogFactor * heightFog, 0.0, 1.0);
        fragColor = vec4(mix(uFogColor, fragColor.rgb, fogFactor), 1.0);
    }
}
