#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "cutter.hpp" 

namespace mini {
	static millable_block::milling_mask_t make_mask(float radius, const millable_block& block) {
		assert(radius > 0.0f && "radius has to be positive");

		auto block_size = block.get_block_size();
		float unit_size_x = block_size.x / block.get_heightmap_width();
		float unit_size_y = block_size.z / block.get_heightmap_height();

		uint32_t mask_width = static_cast<uint32_t>(2*radius / unit_size_x) + 1;
		uint32_t mask_height = static_cast<uint32_t>(2*radius / unit_size_y) + 1;

		millable_block::milling_mask_t mask(mask_width, mask_height);

		float cx = radius;
		float cy = radius;

		for (uint32_t x = 0; x < mask_width; ++x) {
			for (uint32_t y = 0; y < mask_height; ++y) {
				float rx = x * unit_size_x;
				float ry = y * unit_size_y;

				float dx = rx - cx;
				float dy = ry - cy;
				float d = dx * dx + dy * dy;

				if (d <= radius * radius) {
					mask.mask[y * mask_width + x] = 0.0f;
				} else {
					mask.mask[y * mask_width + x] = 1.0f;
				}
			}
		}

		return mask;
	}

	milling_cutter::milling_cutter(std::shared_ptr<shader_program> shader, float radius, const millable_block& block) :
		m_mask(make_mask(radius, block)),
		m_radius(radius),
		m_position(0.0f, -2.5f, 0.0f) {

		m_model = std::make_shared<milling_cutter_model>(shader);
	}

	void milling_cutter::update(const float delta_time, millable_block& block) {
		m_position.x += delta_time * 0.5f;
		m_position.z += delta_time * 0.5f;

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
		block.carve(m_mask, offset_x, offset_y, height, 0.0f);
	}

	void milling_cutter::render(app_context& context) {
		auto world = glm::mat4x4(1.0f);

		world = glm::translate(world, m_position);
		world = glm::scale(world, glm::vec3{ m_radius, 1.0f, m_radius });
		world = glm::rotate(world, 0.5f * glm::pi<float>(), glm::vec3{ 1.0f, 0.0f, 0.0f });

		context.draw(m_model, world);
	}

	milling_cutter_model::milling_cutter_model(std::shared_ptr<shader_program> shader) {
		m_shader = shader;
		m_mesh = triangle_mesh::make_cylinder(1.0f, 10.0f, 100, 100);
	}

	milling_cutter_model::~milling_cutter_model() { }

	void milling_cutter_model::render(app_context& context, const glm::mat4x4& world_matrix) const {
		auto& shader = *m_shader.get();

		const auto& view_matrix = context.get_view_matrix();
		const auto& proj_matrix = context.get_projection_matrix();

		shader.bind();

		context.set_lights(shader);
		shader.set_uniform("u_surface_color", glm::vec3{ 0.81f, 0.35f, 0.77f });
		shader.set_uniform("u_shininess", 2.0f);

		shader.set_uniform("u_world", world_matrix);
		shader.set_uniform("u_view", view_matrix);
		shader.set_uniform("u_projection", proj_matrix);

		m_mesh->draw();
	}
}