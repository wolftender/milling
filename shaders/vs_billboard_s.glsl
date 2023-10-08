#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_center;
uniform vec4 u_color;
uniform vec2 u_size;
uniform vec2 u_resolution;

out vec4 vertex_color;
out vec2 uv;

void main () {
    vec3 scaled_pos = vec3 (
        u_size.x * a_position.x / u_resolution.x, 
        u_size.y * a_position.y / u_resolution.y, 
        a_position.z
    );

    vec3 world_pos = u_center;

    vertex_color = a_color * u_color;
    uv = a_uv;

    gl_Position = u_projection * u_view * vec4 (world_pos, 1.0);
    gl_Position = gl_Position / gl_Position.w;

    gl_Position.xy += scaled_pos.xy;
}