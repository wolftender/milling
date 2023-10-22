#version 330

uniform sampler2D u_heightmap;

struct point_light_t {
    vec3 color;
    vec3 position;

    float intensity;
    float att_const;
    float att_lin;
    float att_sq;
};

uniform point_light_t u_point_lights[10];
uniform vec3 u_ambient;
uniform vec3 u_surface_color;
uniform vec3 u_camera_position;
uniform float u_shininess;
uniform float u_gamma;

in vec3 mv_local_pos;
in vec3 mv_world_pos;

out vec4 output_color;

const vec2 size = vec2(2.0, 0.0);
const vec3 offset = vec3(-1, 0, 1);
const float multiplier = 4.0f;

vec3 normalmapping (vec3 N, vec3 T, vec3 tn) {
	vec3 B = normalize (cross (N, T));
	T = cross (B, N);
	mat3 TBN = transpose (mat3(T, B, N));

	return normalize (TBN * tn);
}

vec4 phong (point_light_t light, vec3 viewer_direction, vec3 light_direction, vec3 normal) {
    vec4 diffuse = vec4 (0.0f);
    vec4 specular = vec4 (0.0f);

    // diffuse color
    float df = max (dot (normal, light_direction), 0.0);
    diffuse = light.intensity * df * vec4 (light.color, 1.0);

    vec3 reflected = normalize (reflect (-light_direction, normal));
    float sf = max (dot (viewer_direction, reflected), 0.0);
    
    if (u_shininess == 0.0) {
        sf = 0.0;
    } else {
        sf = pow (sf, u_shininess);
    }

    specular = light.intensity * sf * vec4 (light.color, 1.0);

    return (diffuse + specular);
}

vec4 gamma_correct (vec4 color, float gamma) {
    vec4 out_color = color;
    out_color.xyz = pow (out_color.xyz, vec3 (1.0 / gamma));

    return out_color;
}

void main () {
    vec2 texel_size = 1.0 / vec2(textureSize(u_heightmap, 0));
    vec2 uv = mv_local_pos.xz + 0.5;

    float s11 = texture(u_heightmap, uv).r;
	float s01 = texture(u_heightmap, uv + offset.xy * texel_size).r;
	float s21 = texture(u_heightmap, uv + offset.zy * texel_size).r;
	float s10 = texture(u_heightmap, uv + offset.yx * texel_size).r;
	float s12 = texture(u_heightmap, uv + offset.yz * texel_size).r;

    vec3 va = normalize (vec3 (size.xy, multiplier * (s21 - s01)));
	vec3 vb = normalize (vec3 (size.yx, multiplier * (s12 - s10)));
	vec3 tn = normalize (cross (va, vb));

	vec3 normal = normalmapping(vec3(0.0, -1.0, 0.0), vec3(1.0, 0.0, 0.0), tn);
    float height = texture(u_heightmap, uv).r;

    vec4 phong_component = vec4 (0.0f);

    for (int i = 0; i < 10; ++i) {
        if (u_point_lights[i].intensity > 0.0) {
            vec3 light_direction = u_point_lights[i].position - mv_world_pos.xyz;
            vec3 viewer_direction = u_camera_position - u_point_lights[i].position;

            float dist = length (light_direction);
            float att_inv = u_point_lights[i].att_const + u_point_lights[i].att_lin * dist + u_point_lights[i].att_sq * dist * dist;

            phong_component += phong (u_point_lights[i], normalize (viewer_direction), normalize (light_direction), normal.xyz) / att_inv;
        }
    }

    output_color = gamma_correct (vec4 (u_ambient * u_surface_color, 1.0) + phong_component, u_gamma);
}