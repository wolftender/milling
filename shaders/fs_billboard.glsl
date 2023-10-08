#version 330

in vec4 vertex_color;
in vec2 uv;

out vec4 output_color;

uniform sampler2D u_sampler;

void main () {
    output_color = vertex_color * texture(u_sampler, uv);
}