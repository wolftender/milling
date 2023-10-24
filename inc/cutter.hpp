#pragma once
#include "millable.hpp"
#include "context.hpp"
#include "mesh.hpp"

namespace mini {
	class milling_cutter_model : public graphics_object {
		private:
			std::unique_ptr<triangle_mesh> m_mesh;
			std::shared_ptr<shader_program> m_shader;

			float m_blade_height;

		public:
			milling_cutter_model(std::shared_ptr<shader_program> shader, float blade_height);
			~milling_cutter_model();

			milling_cutter_model(const milling_cutter_model&) = delete;
			milling_cutter_model& operator=(const milling_cutter_model&) = delete;

			virtual void render(app_context& context, const glm::mat4x4& world_matrix) const override;
	};

	class milling_cutter final {
		private:
			std::shared_ptr<milling_cutter_model> m_model;
			millable_block::milling_mask_t m_mask;

			std::vector<glm::vec3> m_path_points;

			glm::vec3 m_position;
			float m_radius;
			bool m_spherical;

			float m_interpolation_time;
			float m_blade_height;

			int m_current_point;

			// error flags for path segment
			bool m_collision_reported;
			bool m_depth_reported;
			bool m_flat_reported;
			
		public:
			milling_cutter(
				std::shared_ptr<shader_program> shader, 
				std::vector<glm::vec3> path_points,
				float radius, 
				bool spherical, 
				float blade_height,
				const millable_block& block);

			~milling_cutter() = default;

			float get_radius() const;
			bool is_spherical() const;

			void update(const float delta_time, millable_block& block);
			void render(app_context& context);

			void instant(millable_block& block);

		private:
			void m_carve(millable_block& block, bool silent, bool vertical);
	};
}