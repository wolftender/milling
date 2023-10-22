#pragma once
#include "millable.hpp"
#include "context.hpp"
#include "mesh.hpp"

namespace mini {
	class milling_cutter_model : public graphics_object {
		private:
			std::unique_ptr<triangle_mesh> m_mesh;
			std::shared_ptr<shader_program> m_shader;

		public:
			milling_cutter_model(std::shared_ptr<shader_program> shader);
			~milling_cutter_model();

			milling_cutter_model(const milling_cutter_model&) = delete;
			milling_cutter_model& operator=(const milling_cutter_model&) = delete;

			virtual void render(app_context& context, const glm::mat4x4& world_matrix) const override;
	};

	class milling_cutter final {
		private:
			std::shared_ptr<milling_cutter_model> m_model;
			millable_block::milling_mask_t m_mask;

			glm::vec3 m_position;
			float m_radius;
			
		public:
			milling_cutter(
				std::shared_ptr<shader_program> shader, 
				float radius, 
				bool spherical, 
				const millable_block& block);

			~milling_cutter() = default;

			void update(const float delta_time, millable_block& block);
			void render(app_context& context);
	};
}