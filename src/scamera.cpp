#include "scamera.hpp"

namespace mini {
	float anaglyph_camera::get_near () const {
		return m_near;
	}

	float anaglyph_camera::get_far () const {
		return m_far;
	}

	float anaglyph_camera::get_left () const {
		return m_left;
	}

	float anaglyph_camera::get_right () const {
		return m_right;
	}

	float anaglyph_camera::get_bottom () const {
		return m_bottom;
	}

	float anaglyph_camera::get_top () const {
		return m_top;
	}

	float anaglyph_camera::get_x_offset () const {
		return m_x_offset;
	}

	void anaglyph_camera::set_near (float near) {
		m_near = near;
		m_recalculate_projection ();
	}

	void anaglyph_camera::set_far (float far) {
		m_far = far;
		m_recalculate_projection ();
	}

	void anaglyph_camera::set_left (float left) {
		m_left = left;
		m_recalculate_projection ();
	}

	void anaglyph_camera::set_right (float right) {
		m_right = right;
		m_recalculate_projection ();
	}

	void anaglyph_camera::set_bottom (float bottom) {
		m_bottom = bottom;
		m_recalculate_projection ();
	}

	void anaglyph_camera::set_top (float top) {
		m_top = top;
		m_recalculate_projection ();
	}

	void anaglyph_camera::set_x_offset (float offset) {
		m_x_offset = offset;
	}

	anaglyph_camera::anaglyph_camera () {
		m_near = 1.0f;
		m_far = 100.0f;
		m_left = -1.0f;
		m_right = 1.0f;
		m_top = 1.0f;
		m_bottom = -1.0f;

		m_recalculate_projection ();
	}

	anaglyph_camera::anaglyph_camera (const glm::vec3 & position, const glm::vec3 & target) : camera (position, target) {
		m_near = 1.0f;
		m_far = 100.0f;
		m_left = -1.0f;
		m_right = 1.0f;
		m_top = 1.0f;
		m_bottom = -1.0f;

		m_recalculate_projection ();
	}

	const glm::mat4x4 & anaglyph_camera::get_projection_matrix () const {
		return m_projection;
	}

	const glm::mat4x4 & anaglyph_camera::get_projection_inverse () const {
		return m_projection_inv;
	}

	void anaglyph_camera::video_mode_change (const video_mode_t & mode) { }

	void anaglyph_camera::m_recalculate_projection () {
		float m00 = 2.0f * m_near / (m_right - m_left);
		float m11 = 2.0f * m_near / (m_top - m_bottom);
		float m22 = -(m_far + m_near) / (m_far - m_near);
		float m20 = (m_right + m_left) / (m_right - m_left);
		float m21 = (m_top + m_bottom) / (m_top - m_bottom);
		float m32 = (-2.0f * m_near * m_far) / (m_far - m_near);

		m_projection = {
			 m00,  0.0f,  0.0f,  0.0f,
			0.0f,   m11,  0.0f,  0.0f,
			 m20,   m21,   m22, -1.0f,
			0.0f,  0.0f,   m32,  0.0f
		};

		m_projection_inv = glm::inverse (m_projection);
		
		glm::mat4x4 translation {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			m_x_offset, 0.0f, 0.0f, 1.0f
		};

		m_projection = m_projection * translation;
	}
}
