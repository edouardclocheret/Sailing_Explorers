#version 330 core

layout(location = 0) in vec3 obj_space_pos;
layout(location = 1) in vec3 obj_space_normal;
layout(location = 2) in mat4 instance_model;

out vec3 world_space_pos;
out vec3 world_space_normal;

uniform mat4 view_mat;
uniform mat4 proj_mat;

void main() {
    vec4 worldPos = instance_model * vec4(obj_space_pos, 1.0);
    world_space_pos = worldPos.xyz;

    mat3 normal_mat = transpose(inverse(mat3(instance_model)));
    world_space_normal = normalize(normal_mat * obj_space_normal);

    gl_Position = proj_mat * view_mat * worldPos;
}
