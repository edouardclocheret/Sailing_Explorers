#version 330 core

in vec2 uv_out;

uniform sampler2D scene_sampler;
uniform sampler2D texture_sampler;

uniform mat4 ViewProjectionMatrix;
uniform mat4 previousViewProjectionMatrix;

out vec4 fragColor;

void main()
{
        float depth = texture(texture_sampler, uv_out).r;

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

        // blur
        vec4 color = texture(scene_sampler, uv_out);
            vec2 uv = uv_out + velNDC;

            // sample a few along the velocity vector (simple box blur along motion)
            const int samples = 5;
            for (int i = 1; i < samples; ++i) {
                vec4 currentColor = texture(scene_sampler, uv);
                color += currentColor;
                uv += velNDC;
            }
            fragColor = color / float(samples);


}
