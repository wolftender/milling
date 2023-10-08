#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_center;
uniform vec2 u_size;
uniform vec2 u_resolution;

out vec4 vertex_color;
out vec2 uv;

void main () {
    vec3 cam_right = vec3 (u_view[0][0], u_view[1][0], u_view[2][0]);
    vec3 cam_up = vec3 (u_view[0][1], u_view[1][1], u_view[2][1]);

    vec3 scaled_pos = vec3 (u_size.x * a_position.x, u_size.y * a_position.y, a_position.z);
    vec3 world_pos = u_center + cam_right * scaled_pos.x + cam_up * scaled_pos.y;

    vertex_color = a_color;
    uv = a_uv;

    gl_Position = u_projection * u_view * vec4 (world_pos, 1.0);
}