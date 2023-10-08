#include <array>

#include "billboard.hpp"

namespace mini {
	constexpr std::array<float, 12> billboard_vertices = {
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	constexpr std::array<float, 8> billboard_texcoords = {
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
		0.0, 1.0
	};

	constexpr std::array<float, 16> billboard_colors = {
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	constexpr std::array<GLuint, 6> billboard_indices = {
		0, 1, 3,
		1, 2, 3
	};

	const glm::vec2 & billboard_object::get_size () const {
		return m_size;
	}

	const glm::vec4 & billboard_object::get_color_tint () const {
		return m_color_tint;
	}

	void billboard_object::set_size (const glm::vec2 & size) {
		m_size = size;
	}

	void billboard_object::set_color_tint (const glm::vec4 & color_tint) {
		m_color_tint = color_tint;
	}

	billboard_object::billboard_object (std::shared_ptr<shader_program> shader, std::shared_ptr<texture> texture) {
		m_shader = shader;
		m_texture = texture;

		constexpr int num_vertices = static_cast<int> (billboard_vertices.size ()) / 3;
		constexpr GLuint a_position = 0;
		constexpr GLuint a_color = 1;
		constexpr GLuint a_uv = 2;

		m_color_tint = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_size = { 1.0f, 1.0f };
		m_shader = shader;
		m_pos_buffer = m_color_buffer = m_index_buffer = m_uv_buffer = m_vao = 0;

		glGenVertexArrays (1, &m_vao);
		glGenBuffers (1, &m_pos_buffer);
		glGenBuffers (1, &m_color_buffer);
		glGenBuffers (1, &m_index_buffer);
		glGenBuffers (1, &m_uv_buffer);
		glBindVertexArray (m_vao);

		glBindBuffer (GL_ARRAY_BUFFER, m_pos_buffer);
		glBufferData (GL_ARRAY_BUFFER, sizeof (float) * num_vertices * 3, billboard_vertices.data (), GL_STATIC_DRAW);
		glVertexAttribPointer (a_position, 3, GL_FLOAT, false, sizeof (float) * 3, (void *)0);
		glEnableVertexAttribArray (a_position);

		glBindBuffer (GL_ARRAY_BUFFER, m_color_buffer);
		glBufferData (GL_ARRAY_BUFFER, sizeof (float) * num_vertices * 4, billboard_colors.data (), GL_STATIC_DRAW);
		glVertexAttribPointer (a_color, 4, GL_FLOAT, false, sizeof (float) * 4, (void *)0);
		glEnableVertexAttribArray (a_color);

		glBindBuffer (GL_ARRAY_BUFFER, m_uv_buffer);
		glBufferData (GL_ARRAY_BUFFER, sizeof (float) * num_vertices * 2, billboard_texcoords.data (), GL_STATIC_DRAW);
		glVertexAttribPointer (a_uv, 2, GL_FLOAT, false, sizeof (float) * 2, (void *)0);
		glEnableVertexAttribArray (a_uv);

		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (GLuint) * billboard_indices.size (), billboard_indices.data (), GL_STATIC_DRAW);

		glBindVertexArray (static_cast<GLuint>(NULL));
	}

	billboard_object::~billboard_object () {
		glDeleteVertexArrays (1, &m_vao);
		glDeleteBuffers (1, &m_pos_buffer);
		glDeleteBuffers (1, &m_color_buffer);
		glDeleteBuffers (1, &m_index_buffer);
		glDeleteBuffers (1, &m_uv_buffer);
	}

	void billboard_object::render (app_context & context, const glm::mat4x4 & world_matrix) const {
		glBindVertexArray (m_vao);
		glDisable (GL_DEPTH_TEST);

		if (m_texture) {
			m_texture->bind ();
		}

		m_shader->bind ();

		// set uniforms
		const auto & view_matrix = context.get_view_matrix ();
		const auto & proj_matrix = context.get_projection_matrix ();

		float screen_width = static_cast<float> (context.get_video_mode ().get_buffer_width ());
		float screen_height = static_cast<float> (context.get_video_mode ().get_buffer_height ());

		glm::vec4 center = { 0.0f, 0.0f, 0.0f, 1.0f };
		center = world_matrix * center;

		m_shader->set_uniform ("u_size", m_size);
		m_shader->set_uniform ("u_color", m_color_tint);
		m_shader->set_uniform ("u_center", static_cast<glm::vec3> (center));
		m_shader->set_uniform ("u_view", view_matrix);
		m_shader->set_uniform ("u_projection", proj_matrix);
		m_shader->set_uniform ("u_resolution", glm::vec2 (screen_width, screen_height));

		glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (quad_indices.size ()), GL_UNSIGNED_INT, NULL);
		glBindVertexArray (static_cast<GLuint>(NULL));
		glEnable (GL_DEPTH_TEST);
	}
}