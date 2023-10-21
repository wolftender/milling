#pragma once
#include "window.hpp"
#include "context.hpp"
#include "store.hpp"
#include "grid.hpp"
#include "parser.hpp"

namespace mini {
	class application : public app_window {
		private:
			app_context m_context;
			resource_store m_store;

			float m_cam_yaw, m_cam_pitch, m_distance;
			float m_time;
			float m_grid_spacing;
			bool m_grid_enabled;

			int m_last_vp_width, m_last_vp_height;
			bool m_viewport_focus, m_mouse_in_viewport;

			glm::vec3 m_camera_target;
			offset_t m_vp_mouse_offset;

			// objects
			std::shared_ptr<grid_object> m_grid_xz;

		public:
			float get_cam_yaw() const;
			float get_cam_pitch() const;
			float get_cam_distance() const;
			float get_time() const;
			bool is_viewport_focused() const;
			bool is_mouse_in_viewport() const;

			const glm::vec3& get_cam_target() const;
			void set_cam_target(const glm::vec3& target);

			application();

		protected:
			virtual void t_integrate(float delta_time) override;
			virtual void t_render() override;

			virtual void t_on_character(unsigned int code) override;
			virtual void t_on_cursor_pos(double posx, double posy) override;
			virtual void t_on_mouse_button(int button, int action, int mods) override;
			virtual void t_on_key_event(int key, int scancode, int action, int mods) override;
			virtual void t_on_scroll(double offset_x, double offset_y) override;
			virtual void t_on_resize(int width, int height) override;

		private:
			void m_handle_mouse();

			void m_draw_main_menu();
			void m_draw_main_window();
			void m_draw_viewport();
			void m_draw_view_options();
			void m_draw_scene_options();
	};
}