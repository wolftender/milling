#include <cassert>

#include "context.hpp"

namespace mini {
	// basic shaders to render the screen buffer
	static const std::string screen_vertex_source = R"(
		#version 330
		layout (location = 0) in vec3 a_position;
		layout (location = 1) in vec2 a_texcoords;
		out vec2 v_texcoords;
		void main () {
			v_texcoords = a_texcoords;
			gl_Position = vec4 (a_position.xyz, 1.0);
		}
	)";

	static const std::string screen_fragment_source = R"(
		#version 330
		in vec2 v_texcoords;
		layout (location = 0) out vec4 v_color;
		uniform sampler2D u_texture;
		void main () {
			vec4 frag_color = texture (u_texture, v_texcoords);
			v_color = frag_color;
		}
	)";

	app_context::app_context (const video_mode_t & video_mode) {
		m_colorbuffer[0] = 0;
		m_colorbuffer[1] = 0;
		m_framebuffer[0] = 0;
		m_framebuffer[1] = 0;
		m_renderbuffer = 0;
		m_last_queue_index = 0;
		m_quad_buffer[0] = 0;
		m_quad_buffer[1] = 0;
		m_quad_buffer[2] = 0;
		m_quad_vao = 0;

		m_video_mode = video_mode;
		m_switch_mode = false;

		m_camera = std::make_unique<default_camera> ();
		m_camera->video_mode_change (m_video_mode);

		// initialize default screen shader
		m_screen_shader = std::make_unique<shader_program> (screen_vertex_source, screen_fragment_source);
		m_screen_shader->compile ();

		m_ambient = glm::vec3{ 0.2f, 0.2f, 0.2f };
		m_gamma = 0.9f;

		m_init_frame_buffer ();
		m_init_screen_quad ();
	}

	app_context::~app_context () {
		m_destroy_screen_quad ();
		m_destroy_frame_buffer ();
	}

	void app_context::set_camera_pos (const glm::vec3 & position) {
		m_camera->set_position (position);
	}

	void app_context::set_camera_target (const glm::vec3 & target) {
		m_camera->set_target (target);
	}

	void app_context::set_pre_render (render_hook_t hook) {
		m_pre_render = hook;
	}

	void app_context::set_post_render (render_hook_t hook) {
		m_post_render = hook;
	}

	void app_context::clear_pre_render () {
		m_pre_render = nullptr;
	}

	void app_context::clear_post_render () {
		m_post_render = nullptr;
	}

	void app_context::set_video_mode (const video_mode_t & video_mode) {
		m_new_mode = video_mode;
		m_switch_mode = true;
	}

	const video_mode_t & app_context::get_video_mode () const {
		return m_video_mode;
	}

	const GLuint app_context::get_front_buffer () const {
		return m_colorbuffer[front];
	}

	const point_light_t& app_context::get_light(const uint64_t index) const {
		return m_lights[index];
	}

	point_light_t& app_context::get_light(const uint64_t index) {
		return m_lights[index];
	}

	void app_context::set_lights(shader_program& shader) const {
		shader.set_uniform("u_ambient", m_ambient);
		shader.set_uniform("u_camera_position", get_camera().get_position());
		shader.set_uniform("u_gamma", m_gamma);

		for (int i = 0; i < 10; ++i) {
			const auto& light = m_lights[i];
			shader.set_uniform("u_point_lights[" + std::to_string(i) + "].position", light.position);
			shader.set_uniform("u_point_lights[" + std::to_string(i) + "].color", light.color);
			shader.set_uniform("u_point_lights[" + std::to_string(i) + "].intensity", light.intensity);
			shader.set_uniform("u_point_lights[" + std::to_string(i) + "].att_const", light.att_const);
			shader.set_uniform("u_point_lights[" + std::to_string(i) + "].att_lin", light.att_lin);
			shader.set_uniform("u_point_lights[" + std::to_string(i) + "].att_sq", light.att_sq);
		}
	}

	camera & app_context::get_camera () {
		return *m_camera.get ();
	}

	const camera & app_context::get_camera () const {
		return *m_camera.get ();
	}

	std::unique_ptr<camera> app_context::set_camera (std::unique_ptr<camera> camera) {
		auto old_camera = std::move (m_camera);
		m_camera = std::move (camera);

		return std::move (old_camera);
	}

	const glm::mat4x4 & app_context::get_view_matrix () const {
		return m_camera->get_view_matrix ();
	}

	const glm::mat4x4 & app_context::get_projection_matrix () const {
		return m_camera->get_projection_matrix ();
	}

	void app_context::draw (std::weak_ptr<graphics_object> object, glm::mat4x4 world_matrix) {
		if (m_last_queue_index < RENDER_QUEUE_SIZE - 1) {
			m_queue[m_last_queue_index].object = object;
			m_queue[m_last_queue_index].world_matrix = world_matrix;

			m_last_queue_index++;
		}
	}

	void app_context::render (bool clear) {
		// bind the framebuffers
		glBindFramebuffer (GL_FRAMEBUFFER, m_framebuffer[back]);

		// raw render
		display_scene (clear);

		// blit the back buffer on the front buffer
		glBindFramebuffer (GL_READ_FRAMEBUFFER, m_framebuffer[back]);
		glBindFramebuffer (GL_DRAW_FRAMEBUFFER, m_framebuffer[front]);

		glBlitFramebuffer (
			0, 0, m_video_mode.get_buffer_width (), m_video_mode.get_buffer_height (),
			0, 0, m_video_mode.get_buffer_width (), m_video_mode.get_buffer_height (),
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);

		glBindFramebuffer (GL_FRAMEBUFFER, static_cast<GLuint>(NULL));
		glBindFramebuffer (GL_READ_FRAMEBUFFER, static_cast<GLuint>(NULL));
		glBindFramebuffer (GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(NULL));
	}

	void app_context::display (bool present, bool clear) {
		// setup viewport
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glViewport (0, 0, m_video_mode.get_viewport_width (), m_video_mode.get_viewport_height ());

		render (clear);

		// draw the screen quad
		glViewport (0, 0, m_video_mode.get_viewport_width (), m_video_mode.get_viewport_height ());

		glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (present) {
			glBindVertexArray (m_quad_vao);
			glActiveTexture (GL_TEXTURE0);
			glBindTexture (GL_TEXTURE_2D, m_colorbuffer[front]);

			if (m_screen_shader) {
				m_screen_shader->bind ();
			} else {
				throw std::runtime_error ("no screen shader was bound");
			}

			glDrawElements (GL_TRIANGLES, quad_indices.size (), GL_UNSIGNED_INT, 0);
			glBindTexture (GL_TEXTURE_2D, static_cast<GLuint>(NULL));
			glBindVertexArray (static_cast<GLuint>(NULL));
		}

		// try to switch mode
		if (m_switch_mode) {
			m_switch_mode = false;
			m_try_switch_mode ();
		}
	}

	void app_context::display_scene (bool clear) {
		glViewport (0, 0, m_video_mode.get_buffer_width (), m_video_mode.get_buffer_height ());

		// clear screen
		glClearColor (0.15f, 0.15f, 0.15f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render the scene
		if (m_pre_render) {
			m_pre_render (*this);
		}

		for (uint32_t index = 0; index < m_last_queue_index; ++index) {
			auto object_ptr = m_queue[index].object.lock ();
			if (object_ptr) {
				object_ptr->render (*this, m_queue[index].world_matrix);
			}
		}

		if (m_post_render) {
			m_post_render (*this);
		}

		// clear the rendering queue
		// this reset pass has to be done to not persist renderables
		if (clear) {
			for (uint32_t index = 0; index < m_last_queue_index; ++index) {
				m_queue[index].object.reset ();
			}

			m_last_queue_index = 0;
		}
	}

	void app_context::m_try_switch_mode () {
		int32_t old_width = m_video_mode.get_buffer_width ();
		int32_t old_height = m_video_mode.get_buffer_height ();

		int32_t new_width = m_new_mode.get_buffer_width ();
		int32_t new_height = m_new_mode.get_buffer_height ();

		m_video_mode = m_new_mode;

		if (old_width != new_width || old_height != new_height) {
			m_camera->video_mode_change (m_video_mode);

			m_destroy_frame_buffer ();
			m_init_frame_buffer ();
		}
	}

	void app_context::m_init_frame_buffer () {
		// values copied from the video mode
		int32_t render_width = m_video_mode.get_buffer_width ();
		int32_t render_height = m_video_mode.get_buffer_height ();
		int32_t render_samples = 4; // antialiasing samples

		glGenFramebuffers (2, m_framebuffer);
		glBindFramebuffer (GL_FRAMEBUFFER, m_framebuffer[back]);

		// create a texture that will be used for color buffer
		glGenTextures (2, m_colorbuffer);
		glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, m_colorbuffer[back]);
		glTexImage2DMultisample (GL_TEXTURE_2D_MULTISAMPLE, render_samples, GL_RGB, render_width, render_height, GL_TRUE);
		glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(NULL));

		// bind the texture as color attachment
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_colorbuffer[back], 0);

		// create renderbuffer for the framebuffer
		glGenRenderbuffers (1, &m_renderbuffer);
		glBindRenderbuffer (GL_RENDERBUFFER, m_renderbuffer);
		glRenderbufferStorageMultisample (GL_RENDERBUFFER, render_samples, GL_DEPTH24_STENCIL8, render_width, render_height);
		glBindRenderbuffer (GL_RENDERBUFFER, static_cast<GLuint>(NULL));

		// bind the renderbuffer as depth stencil attachment
		glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);

		if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error ("opengl error: back framebuffer is not complete");
		}

		// initialize the screen buffer, i.e. what will be actually drawn
		glBindTexture (GL_TEXTURE_2D, m_colorbuffer[front]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, render_width, render_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture (GL_TEXTURE_2D, static_cast<GLuint>(NULL));

		glBindFramebuffer (GL_FRAMEBUFFER, m_framebuffer[front]);
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorbuffer[front], 0);

		if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error ("opengl error: front framebuffer is not complete");
		}

		glBindTexture (GL_TEXTURE_2D, static_cast<GLuint>(NULL));
		glBindFramebuffer (GL_FRAMEBUFFER, static_cast<GLuint>(NULL));
	}

	void app_context::m_init_screen_quad () {
		const uint32_t position_location = 0;
		const uint32_t texcoord_location = 1;

		glGenVertexArrays (1, &m_quad_vao);
		glBindVertexArray (m_quad_vao);

		glGenBuffers (3, m_quad_buffer);

		// 0 = position buffer (3 attribs per vertex)
		// 1 = texture buffer (2 attribs per vertex)
		// 2 = index buffer

		glBindBuffer (GL_ARRAY_BUFFER, m_quad_buffer[0]);
		glBufferData (GL_ARRAY_BUFFER, sizeof (float) * quad_vertices.size (), quad_vertices.data (), GL_STATIC_DRAW);
		glVertexAttribPointer (position_location, 3, GL_FLOAT, false, 3 * sizeof (float), 0);
		glEnableVertexAttribArray (position_location);

		glBindBuffer (GL_ARRAY_BUFFER, m_quad_buffer[1]);
		glBufferData (GL_ARRAY_BUFFER, sizeof (float) * quad_texcoords.size (), quad_texcoords.data (), GL_STATIC_DRAW);
		glVertexAttribPointer (texcoord_location, 2, GL_FLOAT, false, 2 * sizeof (float), 0);
		glEnableVertexAttribArray (texcoord_location);

		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_quad_buffer[2]);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (GLuint) * quad_indices.size (), quad_indices.data (), GL_STATIC_DRAW);

		glBindVertexArray (static_cast<GLuint>(NULL));
	}

	void app_context::m_destroy_frame_buffer () {
		glDeleteFramebuffers (2, m_framebuffer);
		glDeleteTextures (2, m_colorbuffer);
		glDeleteRenderbuffers (1, &m_renderbuffer);
	}

	void app_context::m_destroy_screen_quad () {
		glDeleteBuffers (3, m_quad_buffer);
		glDeleteVertexArrays (1, &m_quad_vao);
	}
}