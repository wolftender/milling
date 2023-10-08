#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 vertex_color;
out vec3 local_pos;
out vec3 world_pos;
out vec3 view_pos;
out vec3 proj_pos;

void main () {
    vertex_color = a_color;

    vec4 world = u_world * vec4 (100.0 * a_position, 1.0);
    vec4 view = u_view * world;
    vec4 proj = u_projection * view;

    local_pos = a_position;
    world_pos = world.xyz;
    view_pos = view.xyz;
    proj_pos = proj.xyz;

    gl_Position = proj;
}