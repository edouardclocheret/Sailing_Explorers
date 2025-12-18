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

/*
  In GLSL, pow(x, y) is undefined for  or if  and , so be careful of these cases as there may be times where shininess = 0!
  */

// Task 5: declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader
in vec3 world_space_pos;
in vec3 world_space_normal;

// Task 10: declare an out vec4 for your output color
out vec4 fragColor;

// Task 12: declare relevant uniform(s) here, for ambient lighting
uniform float ka;

// Task 13: declare relevant uniform(s) here, for diffuse lighting
uniform float kd;

// Task 14: declare relevant uniform(s) here, for specular lighting
uniform float ks;
uniform vec4 cam_pos;
uniform float n;

// Fog uniforms (added from Final Project Gear Up)
uniform bool uFogEnabled;
uniform int uFogType;       // 0 = linear, 1 = exponential, 2 = exponential squared
uniform float uFogDensity;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uFogColor;


float getfalloff(float theta, float inner, float outer){
    float q = (theta - inner) / (outer -inner);
    return -2.f * q*q*q +3* q*q;
}

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized
    vec3 normal = normalize(world_space_normal);



    fragColor = vec4(ka*cAmbient.x, ka*cAmbient.y,ka*cAmbient.z, 0.0);


    for (int i=0; i<8; i++){
        if (lights_8[i].valid == 0) continue;

        //ambiant

        vec3 to_light;
        float fatt;
        if (lights_8[i].type ==0){
            to_light = normalize(lights_8[i].pos.xyz-world_space_pos);
            float dist = distance(lights_8[i].pos,world_space_pos);
            fatt = min(1.0f, 1.f/(lights_8[i].function.x + dist* lights_8[i].function.y +dist*dist*lights_8[i].function.z));

        }
        else if(lights_8[i].type ==1){
            to_light = normalize(-lights_8[i].dir);
            fatt =1.0f;
        }
        else if (lights_8[i].type ==2){
            to_light = normalize(lights_8[i].pos.xyz-world_space_pos);
            float dist = distance(lights_8[i].pos,world_space_pos);
            fatt = min(1.0f, 1.f/(lights_8[i].function.x + dist* lights_8[i].function.y +dist*dist*lights_8[i].function.z));

            //penumbra
            float theta = acos(dot(normalize(-to_light),normalize(lights_8[i].dir)));

            float inner = lights_8[i].angle -lights_8[i].penumbra;
            float outer = lights_8[i].angle;

            if (abs(theta)<inner){
                fatt *=1.0f;
            }
            else if (abs(theta)<outer){
                fatt *=(1-getfalloff(theta, inner, outer));
            }
            else {
                fatt =0.f;
            }
        }

        vec3 reflected = reflect(-to_light, normal);



        //diffuse
        fragColor+= vec4(fatt* lights_8[i].color.x * kd *cDiffuse.x* clamp(dot(to_light, normal), 0.0, 1.0),
                          fatt* lights_8[i].color.y * kd *cDiffuse.y* clamp(dot(to_light, normal), 0.0, 1.0),
                          fatt* lights_8[i].color.z * kd *cDiffuse.z* clamp(dot(to_light, normal), 0.0, 1.0),
                            0.0) ;

        // and specular
        vec3 cam_dir = normalize(cam_pos.xyz - world_space_pos);
        float dotprod = clamp(dot(reflected, cam_dir), 0.0, 1.0);

        if (!(dotprod<0 || (dotprod==0 &&  n<=0))){
            fragColor += vec4(fatt* lights_8[i].color.x * ks *cSpecular.x*pow(dotprod, n),
                              fatt* lights_8[i].color.y * ks *cSpecular.y*pow(dotprod, n),
                              fatt* lights_8[i].color.z * ks *cSpecular.z*pow(dotprod, n),
                              0.0);
        }

    }

    // Apply fog if enabled
    if (uFogEnabled) {
        float distance = length(cam_pos.xyz - world_space_pos);
        float fogFactor;
        if (uFogType == 0) {
            fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);
        } else if (uFogType == 1) {
            fogFactor = exp(-uFogDensity * distance);
        } else {
            float d = uFogDensity * distance;
            fogFactor = exp(-d * d);
        }
        vec3 finalColor = mix(uFogColor, fragColor.rgb, fogFactor);
        fragColor = vec4(finalColor, 1.0);
    }

    //debugging
    //fragColor = vec4(normal, 1.0);

}

