#version 330

uniform sampler2D u_heightmap;

in vec3 mv_local_pos;

out vec4 output_color;

void main () {
    vec2 uv = mv_local_pos.xz + 0.5;
    float height = texture(u_heightmap, uv).r;

    output_color = vec4(uv.x, uv.y, 1.0, 1.0);
}