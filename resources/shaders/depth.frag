#version 330 core

in vec2 uv_out;
uniform sampler2D depth_sampler;

uniform float near;
uniform float far;


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

        float z = depth * 2.0 - 1.0;
        float linearDepth = (2.0 * near * far) / (far + near - z * (far - near));
        float t = clamp((linearDepth - near) / (far - near), 0.0, 1.0);

        // Heatmap
        vec3 color = gradient_color(t);

        fragColor = vec4(color, 1.0);
}
