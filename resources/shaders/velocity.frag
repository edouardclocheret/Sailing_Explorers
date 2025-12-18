#version 330 core

in vec2 uv_out;
uniform sampler2D depth_sampler;

uniform bool is_velx; // if not, it's vely

uniform mat4 ViewProjectionMatrix;
uniform mat4 previousViewProjectionMatrix;

out vec4 fragColor;

vec3 gradient_color (float t){
    vec3 color;
    if (t < 0.25)      color = mix(vec3(0,0,1), vec3(0,1,1), t/0.25);
    else if (t < 0.5)  color = mix(vec3(0,1,1), vec3(0,1,0), (t-0.25)/0.25);
    else if (t < 0.75) color = mix(vec3(0,1,0), vec3(1,1,0), (t-0.5)/0.25);
    else if (t < 1.0)  color = mix(vec3(1,1,0), vec3(1,0,0), (t-0.75)/0.25);
    color = mix(color, vec3(1.0), smoothstep(0.95, 1.0, t));
    return color;
}


void main()
{
        float depth = texture(depth_sampler, uv_out).r;

        vec4 H = vec4(
            uv_out.x * 2.0 - 1.0,
            (1.0 - uv_out.y) * 2.0 - 1.0,
            depth * 2.0 - 1.0,
            1.0
        );
        vec4 D = ViewProjectionMatrix * H;
        vec4 worldPos = D / D.w;

        vec4 currentClip = ViewProjectionMatrix * worldPos;
        vec4 previousClip = previousViewProjectionMatrix * worldPos;

        vec2 currentNDC = currentClip.xy / currentClip.w;
        vec2 previousNDC = previousClip.xy / previousClip.w;

        vec2 velNDC = (currentNDC - previousNDC) * 0.5;

        // Heatmap
        float speed;
        if (is_velx){
         speed= abs(velNDC.x);
        }
        else {speed= abs(velNDC.y);
        }
        float t = clamp(speed * 50.0, 0.0, 1.0);
        vec3 color = gradient_color(t);
        fragColor = vec4(color, 1.0);

}
