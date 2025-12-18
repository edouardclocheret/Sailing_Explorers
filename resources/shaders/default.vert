#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.
layout(location = 0) in vec3 obj_space_pos;
layout(location = 1) in vec3 obj_space_normal;

// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader

out vec3 world_space_pos;
out vec3 world_space_normal;


// Task 6: declare a uniform mat4 to store model matrix

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

//optimisation
uniform mat3 normal_mat;

// Task 7: declare uniform mat4's for the view and projection matrix


void main() {
    // Task 8: compute the world-space position and normal, then pass them to
    //         the fragment shader using the variables created in task 5
    world_space_pos = vec3(model_mat * vec4(obj_space_pos, 1.0));

    // Recall that transforming normals requires obtaining the inverse-transpose of the model matrix!
    // In projects 5 and 6, consider the performance implications of performing this here.
    //inverse(transpose(mat3(model_mat))) is computed for each vertex but can be computed once before passing it to the gpu
    world_space_normal = normalize(normal_mat * obj_space_normal);

    // Task 9: set gl_Position to the object space position transformed to clip space
    gl_Position = proj_mat * view_mat * vec4(world_space_pos,1.0);
    /* homogenize is done under the hood*/

}
