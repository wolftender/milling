#include "millable.hpp"
#include <iostream>

namespace mini {
	uint32_t millable_block::get_heightmap_width() const {
		return m_heightmap_width;
	}

	uint32_t millable_block::get_heightmap_height() const {
		return m_heightmap_height;
	}

	uint32_t millable_block::get_block_width() const {
		return m_block_width;
	}

	uint32_t millable_block::get_block_height() const {
		return m_block_height;
	}

	bool millable_block::carve(
		const milling_mask_t& mask, 
		int32_t offset_x, 
		int32_t offset_y, 
		float depth, 
		float max_height) {

		std::vector<float> subdata;

		int32_t start_offset_x = 0;
		int32_t start_offset_y = 0;
		int32_t end_offset_x = 0;
		int32_t end_offset_y = 0;

		if (offset_x < 0) {
			start_offset_x = -offset_x;
		}
		
		if (offset_x + mask.width >= m_heightmap_width) {
			end_offset_x = mask.width - (m_heightmap_width - offset_x);
		}

		if (offset_y < 0) {
			start_offset_y = -offset_y;
		}

		if (offset_y + mask.height >= m_heightmap_height) {
			end_offset_y = mask.height - (m_heightmap_height - offset_y);
		}

		int32_t subdata_width = mask.width - start_offset_x - end_offset_x;
		int32_t subdata_height = mask.height - start_offset_y - end_offset_y;

		// out of bounds
		if (subdata_width * subdata_height < 0) {
			return true;
		}

		subdata.resize(subdata_width * subdata_height);

		for (int32_t cx = 0; cx < subdata_width; ++cx) {
			for (int32_t cy = 0; cy < subdata_height; ++cy) {
				int32_t rx = start_offset_x + cx;
				int32_t ry = start_offset_y + cy;

				std::size_t subdata_index = subdata_width * cy + cx;
				std::size_t mask_index = mask.width * ry + rx;
				std::size_t hm_index = m_heightmap_width * (ry + offset_y) + rx + offset_x;

				float mask_val = mask.mask[mask_index];
				float hm_val = m_heightmap[hm_index];

				if (mask_val - depth < hm_val) {
					subdata[subdata_index] = glm::max(mask_val - depth, 0.0f);
					m_heightmap[hm_index] = subdata[subdata_index];
				} else {
					subdata[subdata_index] = m_heightmap[hm_index];
				}
			}
		}

		if (m_texture) {
			glBindTexture(GL_TEXTURE_2D, m_texture);
			glTexSubImage2D(
				GL_TEXTURE_2D,
				0,
				offset_x + start_offset_x,
				offset_y + start_offset_y,
				subdata_width,
				subdata_height,
				GL_RED,
				GL_FLOAT,
				subdata.data());

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		return true;
	}

	void millable_block::carve_silent(
		const milling_mask_t& mask, 
		int32_t offset_x, 
		int32_t offset_y, 
		float depth, 
		float max_height,
		millable_block::milling_result_t& result) {

		int32_t start_offset_x = 0;
		int32_t start_offset_y = 0;
		int32_t end_offset_x = 0;
		int32_t end_offset_y = 0;

		if (offset_x < 0) {
			start_offset_x = -offset_x;
		}

		if (offset_x + mask.width >= m_heightmap_width) {
			end_offset_x = mask.width - (m_heightmap_width - offset_x);
		}

		if (offset_y < 0) {
			start_offset_y = -offset_y;
		}

		if (offset_y + mask.height >= m_heightmap_height) {
			end_offset_y = mask.height - (m_heightmap_height - offset_y);
		}

		int32_t subdata_width = mask.width - start_offset_x - end_offset_x;
		int32_t subdata_height = mask.height - start_offset_y - end_offset_y;

		// out of bounds
		if (subdata_width * subdata_height < 0 || subdata_width > mask.width || subdata_height > mask.height) {
			return;
		}

		bool collides = false;

		for (int32_t cx = 0; cx < subdata_width; ++cx) {
			for (int32_t cy = 0; cy < subdata_height; ++cy) {
				int32_t rx = start_offset_x + cx;
				int32_t ry = start_offset_y + cy;

				std::size_t subdata_index = subdata_width * cy + cx;
				std::size_t mask_index = mask.width * ry + rx;
				std::size_t hm_index = m_heightmap_width * (ry + offset_y) + rx + offset_x;

				float mask_val = mask.mask[mask_index];
				float hm_val = m_heightmap[hm_index];

				if (mask_val - depth + max_height < hm_val) {
					result.collision_error = true;
				}

				if (mask_val - depth < hm_val) {
					m_heightmap[hm_index] = glm::max(mask_val - depth, 0.0f);

					result.depth_error = result.depth_error || (m_heightmap[hm_index] < m_min_height);
					result.was_milled = true;
				}
			}
		}
	}

	void millable_block::refresh_texture() {
		if (m_texture) {
			glBindTexture(GL_TEXTURE_2D, m_texture);
			glTexSubImage2D(
				GL_TEXTURE_2D,
				0, 0, 0, m_heightmap_width, m_heightmap_height,
				GL_RED,
				GL_FLOAT,
				m_heightmap.data());

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void millable_block::set_block_dimensions(uint32_t width, uint32_t height) {
		m_block_width = width;
		m_block_height = height;
		m_heightmap_width = width;
		m_heightmap_height = height;

		m_free_buffers();
		m_init_buffers();
	}

	void millable_block::set_block_size(const glm::vec3& scale) {
		m_block_dimensions = scale;
	}

	void millable_block::set_block_position(const glm::vec3& position) {
		m_block_translation = position;
	}

	const glm::vec3& millable_block::get_block_size() const {
		return m_block_dimensions;
	}

	const glm::vec3& millable_block::get_block_position() const {
		return m_block_translation;
	}

	millable_block::millable_block(
		std::shared_ptr<shader_program> shader, 
		std::shared_ptr<shader_program> wall_shader, 
		uint32_t width, 
		uint32_t height,
		float min_height) :

		m_vao(0), 
		m_buffer_index(0), 
		m_buffer_position(0),
		m_texture(0),
		m_vao_w(0),
		m_buffer_position_w(0),
		m_buffer_index_w(0),
		m_buffer_normal_w(0),
		m_heightmap_width(width),
		m_heightmap_height(height),
		m_block_width(width),
		m_block_height(height),
		m_block_shader(shader),
		m_wall_shader(wall_shader),
		m_block_dimensions(1.0f),
		m_block_translation(0.0f),
		m_min_height(min_height) {

		m_init_buffers();
	}

	millable_block::~millable_block() {
		m_free_buffers();
	}

	void millable_block::render(app_context& context, const glm::mat4x4& world_matrix) const {
		glBindVertexArray(m_vao);

		const auto & view_matrix = context.get_view_matrix();
		const auto & proj_matrix = context.get_projection_matrix();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		auto& shader = *m_block_shader.get();

		shader.bind();
		shader.set_uniform("u_world", world_matrix);
		shader.set_uniform("u_view", view_matrix);
		shader.set_uniform("u_projection", proj_matrix);

		context.set_lights(shader);

		shader.set_uniform("u_surface_color", glm::vec3{ 1.0f, 1.0f, 1.0f });
		shader.set_uniform("u_shininess", 3.0f);

		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		/// WALLS
		glBindVertexArray(m_vao_w);

		auto& wall_shader = *m_wall_shader.get();

		wall_shader.bind();
		wall_shader.set_uniform("u_world", world_matrix);
		wall_shader.set_uniform("u_view", view_matrix);
		wall_shader.set_uniform("u_projection", proj_matrix);

		context.set_lights(wall_shader);

		wall_shader.set_uniform("u_surface_color", glm::vec3{ 1.0f, 1.0f, 1.0f });
		wall_shader.set_uniform("u_shininess", 3.0f);

		glDrawElements(GL_TRIANGLES, m_indices_w.size(), GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void millable_block::m_init_buffers() {
		m_heightmap.resize(m_heightmap_width * m_heightmap_height);
		std::fill(m_heightmap.begin(), m_heightmap.end(), 1.0f);

		// init texture
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_heightmap_width, m_heightmap_height, 0, GL_RED, GL_FLOAT, m_heightmap.data());

		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glGenBuffers(1, &m_buffer_position);
		glGenBuffers(1, &m_buffer_index);

		// use tesselation instead of this?
		const uint32_t num_points_x = m_block_width + 1;
		const uint32_t num_points_y = m_block_height + 1;

		const float step_x = 1.0f / static_cast<float>(m_block_width);
		const float step_y = 1.0f / static_cast<float>(m_block_height);

		m_positions.reserve(num_points_x * num_points_y * 3);

		for (auto px = 0; px < num_points_x; ++px) {
			for (auto py = 0; py < num_points_y; ++py) {
				float pos_x = step_x * px - 0.5f;
				float pos_y = 0.0f;
				float pos_z = step_y * py - 0.5f;

				m_positions.push_back(pos_x);
				m_positions.push_back(pos_y);
				m_positions.push_back(pos_z);
			}
		}

		// prepare indices
		// 6 indices per quad
		m_indices.reserve(m_block_width * m_block_height * 6);

		for (auto px = 0; px < m_block_width; ++px) {
			for (auto py = 0; py < m_block_height; ++py) {
				auto lt = py * num_points_x + px;
				auto lb = (py + 1) * num_points_x + px;
				auto rt = py * num_points_x + px + 1;
				auto rb = (py + 1) * num_points_x + px + 1;

				m_indices.push_back(lt);
				m_indices.push_back(rb);
				m_indices.push_back(rt);
				m_indices.push_back(lt);
				m_indices.push_back(lb);
				m_indices.push_back(rb);
			}
		}

		// setup buffers
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), m_positions.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 3, nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer_index);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);

		m_init_wall_buffers();
	}

	void millable_block::m_init_wall_buffers() {
		glGenVertexArrays(1, &m_vao_w);
		glBindVertexArray(m_vao_w);

		glGenBuffers(1, &m_buffer_position_w);
		glGenBuffers(1, &m_buffer_index_w);
		glGenBuffers(1, &m_buffer_normal_w);

		const uint32_t num_points_x = m_block_width + 1;
		const uint32_t num_points_y = m_block_height + 1;

		const float step_x = 1.0f / static_cast<float>(m_block_width);
		const float step_y = 1.0f / static_cast<float>(m_block_height);

		m_positions_w.reserve((2 * num_points_x + 2 * num_points_y) * 6);
		m_normals_w.reserve((2 * num_points_x + 2 * num_points_y) * 6);
		m_indices_w.reserve((2 * num_points_x + 2 * num_points_y) * 6);
		uint32_t base = 0;
		
		for (uint32_t zm = 0; zm <= 1; ++zm) {
			float norm_dir = zm == 0 ? -1.0f : 1.0f;

			for (uint32_t cx = 0; cx < num_points_x; ++cx) {
				float pos_x = step_x * cx - 0.5f;
				float pos_z = -0.5f + 1.0 * zm;

				m_positions_w.push_back(pos_x);
				m_positions_w.push_back(0.0f);
				m_positions_w.push_back(pos_z);

				m_normals_w.push_back(0.0f);
				m_normals_w.push_back(0.0f);
				m_normals_w.push_back(norm_dir);

				m_positions_w.push_back(pos_x);
				m_positions_w.push_back(-1.0f);
				m_positions_w.push_back(pos_z);

				m_normals_w.push_back(0.0f);
				m_normals_w.push_back(0.0f);
				m_normals_w.push_back(norm_dir);

				if (cx != num_points_x - 1) {
					m_indices_w.push_back(base + 0);
					m_indices_w.push_back(base + 1);
					m_indices_w.push_back(base + 2);

					m_indices_w.push_back(base + 1);
					m_indices_w.push_back(base + 2);
					m_indices_w.push_back(base + 3
					);
				}

				base += 2;
			}
		}

		for (uint32_t xm = 0; xm <= 1; ++xm) {
			float norm_dir = xm == 0 ? -1.0f : 1.0f;

			for (uint32_t cy = 0; cy < num_points_y; ++cy) {
				float pos_z = step_y * cy - 0.5f;
				float pos_x = -0.5f + 1.0 * xm;

				m_positions_w.push_back(pos_x);
				m_positions_w.push_back(0.0f);
				m_positions_w.push_back(pos_z);

				m_normals_w.push_back(norm_dir);
				m_normals_w.push_back(0.0f);
				m_normals_w.push_back(0.0f);

				m_positions_w.push_back(pos_x);
				m_positions_w.push_back(-1.0f);
				m_positions_w.push_back(pos_z);

				m_normals_w.push_back(norm_dir);
				m_normals_w.push_back(0.0f);
				m_normals_w.push_back(0.0f);

				if (cy != num_points_y - 1) {
					m_indices_w.push_back(base + 0);
					m_indices_w.push_back(base + 1);
					m_indices_w.push_back(base + 2);

					m_indices_w.push_back(base + 1);
					m_indices_w.push_back(base + 2);
					m_indices_w.push_back(base + 3
					);
				}

				base += 2;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_position_w);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions_w.size(), m_positions_w.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 3, nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_normal_w);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_normals_w.size(), m_normals_w.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(float) * 3, nullptr);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer_index_w);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indices_w.size(), m_indices_w.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void millable_block::m_free_buffers() {
		if (m_texture) {
			glDeleteTextures(1, &m_texture);
		}

		if (m_vao) {
			glDeleteVertexArrays(1, &m_vao);
		}

		if (m_vao_w) {
			glDeleteVertexArrays(1, &m_vao_w);
		}

		if (m_buffer_position) {
			glDeleteBuffers(1, &m_buffer_position);
		}

		if (m_buffer_index) {
			glDeleteBuffers(1, &m_buffer_index);
		}

		if (m_buffer_index_w) {
			glDeleteBuffers(1, &m_buffer_index_w);
		}

		if (m_buffer_position_w) {
			glDeleteBuffers(1, &m_buffer_position_w);
		}

		if (m_buffer_normal_w) {
			glDeleteBuffers(1, &m_buffer_normal_w);
		}

		m_vao = m_buffer_index = m_buffer_position = m_texture = 0;
		m_vao_w = m_buffer_index_w = m_buffer_position_w = m_buffer_normal_w = 0;
	}
}