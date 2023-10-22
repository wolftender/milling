#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 world_position;
out vec4 world_normal;

void main () {
    world_position = u_world * vec4 (a_position, 1.0);
    world_normal = normalize (u_world * vec4 (a_normal, 0.0));

    gl_Position = u_projection * u_view * world_position;
}