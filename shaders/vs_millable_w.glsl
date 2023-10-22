#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform sampler2D u_heightmap;

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 mv_local_pos;
out vec4 world_position;
out vec4 world_normal;

void main () {
    mv_local_pos = a_position;

    vec2 uv = a_position.xz + 0.5;
    float height = texture(u_heightmap, uv).r;

    vec4 mapped_position = vec4(a_position, 1.0);
    mapped_position.y *= height;

    world_position = u_world * mapped_position;
    world_normal = normalize (u_world * vec4 (a_normal, 0.0));

    gl_Position = u_projection * u_view * world_position;
}