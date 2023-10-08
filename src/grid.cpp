#include "grid.hpp"

namespace mini {
	constexpr std::array<float, 12> plane_positions = {
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, -1.0f,
		-1.0f, 0.0f, -1.0f,
		-1.0f, 0.0f, 1.0f
	};

	constexpr std::array<float, 16> plane_colors = {
		1, 0, 0, 1,
		0, 1, 0, 1,
		0, 0, 1, 1,
		1, 0, 0, 1
	};

	constexpr std::array<GLuint, 6> plane_indices = {
		0, 1, 3,
		1, 2, 3
	};

	void grid_object::set_spacing (float spacing) {
		m_spacing = spacing;
	}

	float grid_object::get_spacing () const {
		return m_spacing;
	}

	grid_object::grid_object (std::shared_ptr<shader_program> shader) {
		constexpr int num_vertices = 4;
		constexpr GLuint a_position = 0;
		constexpr GLuint a_color = 1;

		m_shader = shader;
		m_pos_buffer = m_color_buffer = m_index_buffer = m_vao = 0;
		m_spacing = 1.0f;

		glGenVertexArrays (1, &m_vao);
		glGenBuffers (1, &m_pos_buffer);
		glGenBuffers (1, &m_color_buffer);
		glGenBuffers (1, &m_index_buffer);
		glBindVertexArray (m_vao);

		glBindBuffer (GL_ARRAY_BUFFER, m_pos_buffer);
		glBufferData (GL_ARRAY_BUFFER, sizeof (float) * num_vertices * 3, plane_positions.data (), GL_STATIC_DRAW);
		glVertexAttribPointer (a_position, 3, GL_FLOAT, false, sizeof (float) * 3, (void *)0);
		glEnableVertexAttribArray (a_position);

		glBindBuffer (GL_ARRAY_BUFFER, m_color_buffer);
		glBufferData (GL_ARRAY_BUFFER, sizeof (float) * num_vertices * 4, plane_colors.data (), GL_STATIC_DRAW);
		glVertexAttribPointer (a_color, 4, GL_FLOAT, false, sizeof (float) * 4, (void *)0);
		glEnableVertexAttribArray (a_color);

		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (GLuint) * plane_indices.size (), plane_indices.data (), GL_STATIC_DRAW);

		glBindVertexArray (static_cast<GLuint>(NULL));
	}

	grid_object::~grid_object () {
		glDeleteVertexArrays (1, &m_vao);
		glDeleteBuffers (1, &m_pos_buffer);
		glDeleteBuffers (1, &m_color_buffer);
		glDeleteBuffers (1, &m_index_buffer);
	}

	void grid_object::render (app_context & context, const glm::mat4x4 & world_matrix) const {
		glBindVertexArray (m_vao);

		m_shader->bind ();

		// set uniforms
		const auto & view_matrix = context.get_view_matrix ();
		const auto & proj_matrix = context.get_projection_matrix ();

		//const auto & view_matrix = make_identity ();
		//const auto& proj_matrix = make_identity ();

		m_shader->set_uniform ("u_world", world_matrix);
		m_shader->set_uniform ("u_view", view_matrix);
		m_shader->set_uniform ("u_projection", proj_matrix);
		m_shader->set_uniform ("u_grid_spacing", m_spacing);
		m_shader->set_uniform ("u_focus_position", context.get_camera ().get_target ());

		glDrawElements (GL_TRIANGLES, plane_indices.size (), GL_UNSIGNED_INT, NULL);
		glBindVertexArray (static_cast<GLuint>(NULL));
		glBindVertexArray (static_cast<GLuint>(NULL));
	}
}
