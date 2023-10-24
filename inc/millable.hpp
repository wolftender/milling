#pragma once
#include "context.hpp"

namespace mini {
	class millable_block : public graphics_object {
		public:
			struct milling_mask_t {
				std::vector<float> mask;

				uint32_t width;
				uint32_t height;

				milling_mask_t(uint32_t width, uint32_t height) : 
					width(width),
					height(height),
					mask(width * height) { }
			};

			struct milling_result_t {
				bool collision_error;
				bool depth_error;
				bool was_milled;

				milling_result_t() : collision_error(false), depth_error(false), was_milled(false) { }
			};

		private:
			std::vector<float> m_heightmap;

			uint32_t m_heightmap_width;
			uint32_t m_heightmap_height;

			uint32_t m_block_width;
			uint32_t m_block_height;

			glm::vec3 m_block_dimensions;
			glm::vec3 m_block_translation;

			GLuint m_vao;
			GLuint m_texture;
			GLuint m_buffer_position, m_buffer_index;

			GLuint m_vao_w;
			GLuint m_buffer_position_w, m_buffer_index_w, m_buffer_normal_w;

			std::vector<float> m_positions;
			std::vector<float> m_positions_w;
			std::vector<float> m_normals_w;

			std::vector<uint32_t> m_indices;
			std::vector<uint32_t> m_indices_w;

			std::shared_ptr<shader_program> m_block_shader;
			std::shared_ptr<shader_program> m_wall_shader;

			float m_min_height;

		public:
			uint32_t get_heightmap_width() const;
			uint32_t get_heightmap_height() const;

			uint32_t get_block_width() const;
			uint32_t get_block_height() const;

			bool carve(
				const milling_mask_t& mask, 
				int32_t offset_x, 
				int32_t offset_y, 
				float depth, 
				float max_height);

			void carve_silent(
				const milling_mask_t& mask,
				int32_t offset_x,
				int32_t offset_y,
				float depth,
				float max_height,
				milling_result_t& result);

			void refresh_texture();

			void set_block_dimensions(uint32_t width, uint32_t height);

			void set_block_size(const glm::vec3 & scale);
			void set_block_position(const glm::vec3 & position);

			const glm::vec3 & get_block_size() const;
			const glm::vec3 & get_block_position() const;

			millable_block(
				std::shared_ptr<shader_program> shader, 
				std::shared_ptr<shader_program> wall_shader,
				uint32_t width, 
				uint32_t height,
				float min_height);

			~millable_block();

			millable_block(const millable_block&) = delete;
			millable_block& operator=(const millable_block&) = delete;

			virtual void render(app_context& context, const glm::mat4x4& world_matrix) const override;

		private:
			void m_init_buffers();
			void m_init_wall_buffers();

			void m_free_buffers();
	};
}