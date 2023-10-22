#version 330

layout (location = 0) in vec3 a_position;

uniform sampler2D u_heightmap;

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 mv_local_pos;

void main () {
    mv_local_pos = a_position;

    vec2 uv = a_position.xz + 0.5;
    float height = texture(u_heightmap, uv).r;

    vec4 mapped_position = vec4(a_position, 1.0);
    mapped_position.y *= height;

    gl_Position = u_projection * u_view * u_world * mapped_position;
}