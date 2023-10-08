#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

uniform mat4 u_world;
uniform vec4 u_color;
uniform vec2 u_size;
uniform vec2 u_resolution;

out vec4 vertex_color;
out vec2 uv;

void main () {
    vec3 local_pos = a_position.xyz;
    local_pos.x = local_pos.x * u_size.x * 0.5f;
    local_pos.y = local_pos.y * u_size.y * 0.5f;

    vec3 screen_pos = (u_world * vec4 (local_pos, 1.0)).xyz;

    vec2 ndc_pos = vec2 (
        2.0f * (screen_pos.x / u_resolution.x) - 1.0f,
        2.0f * (screen_pos.y / u_resolution.y) - 1.0f
    );

    vertex_color = a_color * u_color;
    uv = a_uv;

    gl_Position = vec4 (ndc_pos, 0.0, 1.0);
}