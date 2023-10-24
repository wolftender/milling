#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "cutter.hpp" 

namespace mini {
	static millable_block::milling_mask_t make_mask(float radius, bool spherical, const millable_block& block) {
		assert(radius > 0.0f && "radius has to be positive");

		auto block_size = block.get_block_size();
		float unit_size_x = block_size.x / block.get_heightmap_width();
		float unit_size_y = block_size.z / block.get_heightmap_height();
		float unit_size_z = 1.0f / block_size.y;

		uint32_t mask_width = static_cast<uint32_t>(2*radius / unit_size_x) + 1;
		uint32_t mask_height = static_cast<uint32_t>(2*radius / unit_size_y) + 1;

		millable_block::milling_mask_t mask(mask_width, mask_height);

		float cx = radius;
		float cy = radius;
		float rr = radius * radius;

		for (uint32_t x = 0; x < mask_width; ++x) {
			for (uint32_t y = 0; y < mask_height; ++y) {
				float rx = x * unit_size_x;
				float ry = y * unit_size_y;

				float dx = rx - cx;
				float dy = ry - cy;
				float d = dx * dx + dy * dy;

				if (d <= radius * radius) {
					if (spherical) {
						mask.mask[y * mask_width + x] = (radius - sqrtf(rr - d)) * unit_size_z;
					} else {
						mask.mask[y * mask_width + x] = 0.0f;
					}
				} else {
					mask.mask[y * mask_width + x] = 1.0f;
				}
			}
		}

		return mask;
	}

	milling_cutter::milling_cutter(
		std::shared_ptr<shader_program> shader, 
		std::vector<glm::vec3> path_points,
		float radius, 
		bool spherical, 
		float blade_height,
		const millable_block& block) :

		m_mask(make_mask(radius, spherical, block)),
		m_radius(radius),
		m_path_points(path_points),
		m_interpolation_time(0.0f),
		m_current_point(0),
		m_blade_height(blade_height),
		m_spherical(spherical),
		m_position(0.0f, -2.5f, 0.0f) {

		m_collision_reported = false;
		m_depth_reported = false;
		m_flat_reported = false;

		m_model = std::make_shared<milling_cutter_model>(shader, blade_height);
	}

	float milling_cutter::get_radius() const {
		return m_radius;
	}

	bool milling_cutter::is_spherical() const {
		return m_spherical;
	}

	void milling_cutter::update(const float delta_time, millable_block& block) {
		m_interpolation_time += delta_time;

		if (m_current_point < m_path_points.size() - 1) {
			const float step = m_radius * 0.025f;

			while (m_current_point < m_path_points.size() - 1) {
				auto pos_start = m_path_points[m_current_point];
				auto pos_end = m_path_points[m_current_point + 1];
				bool is_vertical = abs(pos_start.y - pos_end.y) > 0.0001f;

				float len = glm::distance(pos_start, pos_end);
				float t = m_interpolation_time / len;

				float m = glm::min(1.0f, t);
				while (m > step) {
					m = m - step;

					m_position = glm::mix(pos_start, pos_end, glm::min(1.0f, t - m));
					m_carve(block, true, is_vertical);
				}

				m_position = glm::mix(pos_start, pos_end, glm::min(1.0f, t));
				m_carve(block, true, is_vertical);

				if (t > 1.0f) {
					m_interpolation_time -= len;
					m_current_point++;

					m_collision_reported = false;
					m_depth_reported = false;
					m_flat_reported = false;
				} else {
					break;
				}
			}

			block.refresh_texture();
		} else {
			m_position = m_path_points.back();
		}
	}

	void milling_cutter::render(app_context& context) {
		auto world = glm::mat4x4(1.0f);

		world = glm::translate(world, m_position);
		world = glm::scale(world, glm::vec3{ m_radius, 1.0f, m_radius });
		world = glm::rotate(world, 0.5f * glm::pi<float>(), glm::vec3{ 1.0f, 0.0f, 0.0f });

		context.draw(m_model, world);
	}

	void milling_cutter::instant(millable_block& block) {
		const float step = m_radius * 0.025f;

		while (m_current_point < m_path_points.size() - 1) {
			std::cout << "[INFO] complete paths " << m_current_point << " out of " << m_path_points.size() - 1 << std::endl;

			auto pos_start = m_path_points[m_current_point];
			auto pos_end = m_path_points[m_current_point + 1];
			bool is_vertical = abs(pos_start.y - pos_end.y) > 0.0001f;

			float len = glm::distance(pos_start, pos_end);
			float t = 1.0f, m = 1.0f;

			float s = step / len;

			while (m > s) {
				m = m - s;

				m_position = glm::mix(pos_start, pos_end, glm::min(1.0f, t - m));
				m_carve(block, true, is_vertical);
			}

			m_position = glm::mix(pos_start, pos_end, glm::min(1.0f, t));
			m_carve(block, true, is_vertical);

			m_current_point++;

			m_collision_reported = false;
			m_depth_reported = false;
			m_flat_reported = false;
		}

		block.refresh_texture();
		m_position = m_path_points.back();
	}

	void milling_cutter::m_carve(millable_block& block, bool silent, bool vertical) {
		// calculate the offset
		auto block_size = block.get_block_size();
		float unit_size_x = block_size.x / block.get_heightmap_width();
		float unit_size_y = block_size.z / block.get_heightmap_height();

		int32_t mask_width = static_cast<int32_t>(m_radius / unit_size_x);
		int32_t mask_height = static_cast<int32_t>(m_radius / unit_size_y);

		auto block_position = block.get_block_position();
		float relative_x = m_position.x - block_position.x + block_size.x * 0.5f - m_radius;
		float relative_y = m_position.z - block_position.z + block_size.z * 0.5f - m_radius;

		int32_t offset_x = static_cast<int32_t>(relative_x / unit_size_x);
		int32_t offset_y = static_cast<int32_t>(relative_y / unit_size_y);

		float height = (m_position.y - block_position.y) / block_size.y;
		millable_block::milling_result_t result;

		if (silent) {
			block.carve_silent(m_mask, offset_x, offset_y, height, m_blade_height / block_size.y, result);
		} else {
			block.carve(m_mask, offset_x, offset_y, height, m_blade_height / block_size.y);
		}

		if (result.collision_error && !m_collision_reported) {
			m_collision_reported = true;
			std::cerr << "[ERROR] collision reported on path segment " << m_current_point << "!" << std::endl;
		}

		if (result.depth_error && !m_depth_reported) {
			m_depth_reported = true;
			std::cerr << "[ERROR] milling too deep reported on path segment " << m_current_point << "!" << std::endl;
		}

		if (!m_flat_reported && result.was_milled && vertical && !m_spherical) {
			m_flat_reported = true;
			std::cerr << "[ERROR] vertical milling with flat cutter on path segment " << m_current_point << "!" << std::endl;
		}
	}

	milling_cutter_model::milling_cutter_model(std::shared_ptr<shader_program> shader, float blade_height) {
		m_shader = shader;
		m_blade_height = blade_height;
		m_mesh = triangle_mesh::make_cylinder(1.0f, 10.0f, 100, 100);
	}

	milling_cutter_model::~milling_cutter_model() { }

	void milling_cutter_model::render(app_context& context, const glm::mat4x4& world_matrix) const {
		auto& shader = *m_shader.get();

		const auto& view_matrix = context.get_view_matrix();
		const auto& proj_matrix = context.get_projection_matrix();

		const auto blade_offset = glm::translate(glm::mat4x4(1.0f), { 0.0f, -m_blade_height, 0.0f });
		const auto blade_scale = glm::scale(glm::mat4x4(1.0f), { 1.0f, 1.0f, m_blade_height / 10.0f });

		shader.bind();

		context.set_lights(shader);
		shader.set_uniform("u_surface_color", glm::vec3{ 1.0f, 0.2f, 0.0f });
		shader.set_uniform("u_shininess", 1.0f);

		shader.set_uniform("u_world", world_matrix * blade_scale);
		shader.set_uniform("u_view", view_matrix);
		shader.set_uniform("u_projection", proj_matrix);

		m_mesh->draw();

		shader.set_uniform("u_surface_color", glm::vec3{ 0.81f, 0.35f, 0.77f });
		shader.set_uniform("u_world", blade_offset * world_matrix);

		m_mesh->draw();
	}
}