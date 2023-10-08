#pragma once
#include <memory>
#include <array>
#include <functional>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "camera.hpp"

namespace mini {
	class app_context;

	constexpr uint32_t back = 0;
	constexpr uint32_t front = 1;

	const static std::array<float, 12> quad_vertices = {
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	const static std::array<float, 8> quad_texcoords = {
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
		0.0, 1.0
	};

	const static std::array<GLuint, 6> quad_indices = {
		0, 1, 3,
		1, 2, 3
	};

	/// <summary>
	/// This interface represents a visible object on the scene.
	/// </summary>
	class graphics_object {
		public:
			virtual ~graphics_object () { }
			virtual void render (app_context & context, const glm::mat4x4 & world_matrix) const = 0;
	};

	constexpr const uint64_t RENDER_QUEUE_SIZE = 1024;

	/// <summary>
	/// This class represents the graphics context. Because the application is
	/// object oriented and opengl is procedural, we want an object oriented wrapper.
	/// </summary>
	class app_context final {
		public:
			using render_hook_t = std::function<void (app_context &)>;

		private:
			struct enqueued_renderable_t {
				std::weak_ptr<graphics_object> object;
				glm::mat4x4 world_matrix;
			};

			std::array<enqueued_renderable_t, RENDER_QUEUE_SIZE> m_queue;
			uint64_t m_last_queue_index;

			// opengl framebuffer objects
			GLuint m_framebuffer[2], m_colorbuffer[2];
			GLuint m_renderbuffer;

			// screen quad
			GLuint m_quad_vao, m_quad_buffer[3];
			std::unique_ptr<shader_program> m_screen_shader;

			video_mode_t m_video_mode, m_new_mode;
			bool m_switch_mode;

			std::unique_ptr<camera> m_camera;

			render_hook_t m_pre_render;
			render_hook_t m_post_render;

		public:
			app_context (const video_mode_t & video_mode);
			~app_context ();

			app_context (const app_context &) = delete;
			app_context & operator= (const app_context &) = delete;

			void set_camera_pos (const glm::vec3 & position);
			void set_camera_target (const glm::vec3 & target);

			void set_pre_render (render_hook_t hook);
			void set_post_render (render_hook_t hook);
			void clear_pre_render ();
			void clear_post_render ();

			void set_video_mode (const video_mode_t & video_mode);

			const video_mode_t & get_video_mode () const;
			const GLuint get_front_buffer () const;

			camera & get_camera ();
			const camera & get_camera () const;

			std::unique_ptr<camera> set_camera (std::unique_ptr<camera> camera);

			const glm::mat4x4 & get_view_matrix () const;
			const glm::mat4x4 & get_projection_matrix () const;

			void draw (std::weak_ptr<graphics_object> object, glm::mat4x4 world_matrix);
			void render (bool clear);
			void display (bool present, bool clear);
			void display_scene (bool clear);

		private:
			void m_try_switch_mode ();

			void m_init_frame_buffer ();
			void m_init_screen_quad ();

			void m_destroy_frame_buffer ();
			void m_destroy_screen_quad ();
	};
}