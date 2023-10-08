#pragma once
#include "camera.hpp"

namespace mini {
	class anaglyph_camera : public camera {
		private:
			float m_near, m_far, m_left, m_right, m_top, m_bottom;
			float m_x_offset;

			glm::mat4x4 m_projection;
			glm::mat4x4 m_projection_inv;

		public:
			float get_near () const;
			float get_far () const;
			float get_left () const;
			float get_right () const;
			float get_bottom () const;
			float get_top () const;
			float get_x_offset () const;

			void set_near (float near);
			void set_far (float far);
			void set_left (float left);
			void set_right (float right);
			void set_bottom (float bottom);
			void set_top (float top);
			void set_x_offset (float offset);

			anaglyph_camera ();
			anaglyph_camera (const glm::vec3 & position, const glm::vec3 & target);

			virtual const glm::mat4x4 & get_projection_matrix () const override;
			virtual const glm::mat4x4 & get_projection_inverse () const override;
			virtual void video_mode_change (const video_mode_t & mode) override;

		private:
			void m_recalculate_projection ();
	};
}