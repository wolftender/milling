#version 330

layout (location = 0) in vec3 a_position;

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_projection;

void main () {
    gl_Position = u_projection * u_view * u_world * vec4 (a_position, 1.0);
}