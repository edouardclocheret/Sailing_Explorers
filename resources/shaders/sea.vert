#version 330 core

layout(location = 0) in vec3 obj_space_pos;
layout(location = 1) in vec3 obj_space_normal;
layout(location = 2) in vec2 uv;

out vec3 world_space_pos;
out vec3 world_space_normal;
out vec2 uv_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_mat;

void main() {
    uv_out = uv;

    world_space_pos    = vec3(model * vec4(obj_space_pos, 1.0));
    world_space_normal = normalize(normal_mat * obj_space_normal);

    gl_Position = projection * view * vec4(world_space_pos, 1.0);
}
