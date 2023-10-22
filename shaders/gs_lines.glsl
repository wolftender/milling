#version 330

layout (lines) in;
layout (triangle_strip, max_vertices = 16) out;

// screen resolution
uniform vec2 u_resolution;
uniform float u_line_width;

void make_line (vec4 p1, vec4 p2) {
    // screen space ndc
    vec3 s1 = p1.xyz / p1.w;
    vec3 s2 = p2.xyz / p2.w;

    // line "coordinates" offset
    // line is a quad that is kind of like skewed in the direction
    // perpendicular to the tangent of the line
    vec2 line_forward = normalize (s2.xy - s1.xy);
    vec2 line_right = vec2 (-line_forward.y, line_forward.x);
    vec2 line_offset = (vec2 (u_line_width) / u_resolution) * line_right;

    // this is not the end of the line, so we just emit two first vertices
    // the next two are expected to be emitted by next calls
    gl_Position = vec4(p1.xy + line_offset * p1.w, p1.zw);
    EmitVertex();

    gl_Position = vec4(p1.xy - line_offset * p1.w, p1.zw);
    EmitVertex();

    gl_Position = vec4(p2.xy + line_offset * p2.w, p2.zw);
    EmitVertex();

    gl_Position = vec4(p2.xy - line_offset * p2.w, p2.zw);
    EmitVertex();
}

void main () {
    // polynomial control points
    vec4 b0 = gl_in[0].gl_Position;
    vec4 b1 = gl_in[1].gl_Position;

    make_line (b0, b1);
    EndPrimitive();
}