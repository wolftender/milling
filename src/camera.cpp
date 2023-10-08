#include "camera.hpp"

namespace mini {
	int32_t video_mode_t::get_viewport_width () const {
		return m_viewport_width;
	}

	int32_t video_mode_t::get_viewport_height () const {
		return m_viewport_height;
	}

	int32_t video_mode_t::get_buffer_width () const {
		return m_buffer_width;
	}

	int32_t video_mode_t::get_buffer_height () const {
		return m_buffer_height;
	}

	void video_mode_t::set_viewport_width (int32_t vp_width) {
		assert (vp_width >= 8 && vp_width <= 8192);
		m_viewport_width = vp_width;
	}

	void video_mode_t::set_viewport_height (int32_t vp_height) {
		assert (vp_height > 8 && vp_height <= 8192);
		m_viewport_height = vp_height;
	}

	void video_mode_t::set_buffer_width (int32_t buf_width) {
		assert (buf_width >= 8 && buf_width <= 8192);
		m_buffer_width = buf_width;
	}

	void video_mode_t::set_buffer_height (int32_t buf_height) {
		assert (buf_height > 8 && buf_height <= 8192);
		m_buffer_height = buf_height;
	}

	video_mode_t::video_mode_t () {
		set_viewport_width (640);
		set_viewport_height (480);
		set_buffer_width (640);
		set_buffer_height (480);
	}

	video_mode_t::video_mode_t (int32_t width, int32_t height) {
		set_viewport_width (width);
		set_viewport_height (height);
		set_buffer_width (width);
		set_buffer_height (height);
	}

	video_mode_t::video_mode_t (int32_t vp_width, int32_t vp_height, int32_t buf_width, int32_t buf_height) {
		set_viewport_width (vp_width);
		set_viewport_height (vp_height);
		set_buffer_width (buf_width);
		set_buffer_height (buf_height);
	}

	//////////////////////////////////////////////////

	const glm::mat4x4 & camera::get_view_matrix () const {
		return m_view;
	}

	const glm::mat4x4 & camera::get_view_inverse () const {
		return m_view_inv;
	}

	const glm::vec3 & camera::get_position () const {
		return m_position;
	}

	const glm::vec3 & camera::get_target () const {
		return m_target;
	}

	void camera::set_position (const glm::vec3 & position) {
		m_position = position;
		m_recalculate_view ();
	}

	void camera::set_target (const glm::vec3 & target) {
		m_target = target;
		m_recalculate_view ();
	}

	camera::camera () {
		m_position = { 0.0f, 0.0f, 1.0f };
		m_target = { 0.0f, 0.0f, 0.0f };
		
		m_recalculate_view ();
	}

	camera::camera (const glm::vec3 & position, const glm::vec3 & target) {
		m_position = position;
		m_target = target;

		m_recalculate_view ();
	}

	void camera::m_recalculate_view () {
		glm::vec3 dir = m_position - m_target;
		glm::vec3 up = { 0.0f, 1.0f, 0.0f };

		glm::vec3 f = glm::normalize (dir);
		glm::vec3 r = glm::normalize (glm::cross (up, f));
		glm::vec3 u = glm::normalize (glm::cross (f, r));

		m_view = {
			r[0], u[0], f[0], 0.0f,
			r[1], u[1], f[1], 0.0f,
			r[2], u[2], f[2], 0.0f,
			-glm::dot (r, m_position), -glm::dot (u, m_position), -glm::dot (f, m_position), 1.0f
		};

		m_view_inv = glm::inverse (m_view);

		/*m_view = glm::lookAt (m_position, m_target, {0.0f, 1.0f, 0.0f});
		m_view_inv = glm::inverse (m_view);*/
	}

	//////////////////////////////////////////////////

	float default_camera::get_fov () const {
		return m_fov;
	}

	float default_camera::get_near () const {
		return m_near;
	}

	float default_camera::get_far () const {
		return m_far;
	}

	float default_camera::get_aspect () const {
		return m_aspect;
	}

	const glm::mat4x4 & default_camera::get_projection_matrix () const {
		return m_projection;
	}

	const glm::mat4x4 & default_camera::get_projection_inverse () const {
		return m_projection_inv;
	}

	void default_camera::video_mode_change (const video_mode_t & mode) {
		float width = mode.get_buffer_width ();
		float height = mode.get_buffer_height ();

		set_aspect (width / height);
	}

	void default_camera::set_fov (float fov) {
		m_fov = fov;
		m_recalculate_projection ();
	}

	void default_camera::set_near (float near) {
		m_near = near;
		m_recalculate_projection ();
	}

	void default_camera::set_far (float far) {
		m_far = far;
		m_recalculate_projection ();
	}

	void default_camera::set_aspect (float aspect) {
		m_aspect = aspect;
		m_recalculate_projection ();
	}

	default_camera::default_camera () {
		m_aspect = 1.0f;
		m_far = 100.0f;
		m_near = 0.1f;
		m_fov = 3.141592f / 3.0f;

		m_recalculate_projection ();
	}

	default_camera::default_camera (const glm::vec3 & position, const glm::vec3 & target) : camera (position, target) {
		m_aspect = 1.0f;
		m_far = 100.0f;
		m_near = 0.1f;
		m_fov = 3.141592f / 3.0f;

		m_recalculate_projection ();
	}

	void default_camera::m_recalculate_projection () {
		float t = glm::tan (m_fov / 2.0f);
		float a = m_aspect;
		float zm = m_far - m_near;
		float zp = m_far + m_near;

		m_projection = {
			1.0f / (t * a), 0.0f,     0.0f,     0.0f,
			0.0f,           1.0f / t, 0.0f,     0.0f,
			0.0f,           0.0f,     -zp / zm, -1.0f,
			0.0f,           0.0f,     -(2.0f * m_far * m_near) / zm,    0.0f
		};

		m_projection_inv = glm::inverse (m_projection);

		/*m_projection = glm::perspective (m_fov, m_aspect, m_near, m_far);
		m_projection_inv = glm::inverse (m_projection);*/
	}
}