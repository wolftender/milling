#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 vertex_color;

void main () {
    vertex_color = a_color;
    gl_Position = u_projection * u_view * u_world * vec4 (a_position, 1.0);
}