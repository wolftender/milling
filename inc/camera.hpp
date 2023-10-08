#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace mini {
	/// <summary>
	/// This class represents the video mode of the context. Add context settings here.
	/// </summary>
	class video_mode_t {
		private:
			int32_t m_viewport_width, m_viewport_height;
			int32_t m_buffer_width, m_buffer_height;

		public:
			int32_t get_viewport_width () const;
			int32_t get_viewport_height () const;
			int32_t get_buffer_width () const;
			int32_t get_buffer_height () const;

			void set_viewport_width (int32_t vp_width);
			void set_viewport_height (int32_t vp_height);
			void set_buffer_width (int32_t buf_width);
			void set_buffer_height (int32_t buf_height);

			video_mode_t ();
			video_mode_t (int32_t width, int32_t height);
			video_mode_t (int32_t vp_width, int32_t vp_height, int32_t buf_width, int32_t buf_height);

			video_mode_t (const video_mode_t &) = default;
			video_mode_t & operator= (const video_mode_t &) = default;
			~video_mode_t () = default;
	};

	class camera {
		private:
			glm::vec3 m_position, m_target;
			glm::mat4x4 m_view, m_view_inv;

		public:
			const glm::mat4x4 & get_view_matrix () const;
			const glm::mat4x4 & get_view_inverse () const;
			
			virtual const glm::mat4x4 & get_projection_matrix () const = 0;
			virtual const glm::mat4x4 & get_projection_inverse () const = 0;
			virtual void video_mode_change (const video_mode_t & mode) = 0;

			const glm::vec3 & get_position () const;
			const glm::vec3 & get_target () const;

			void set_position (const glm::vec3 & position);
			void set_target (const glm::vec3 & target);

			camera ();
			camera (const glm::vec3 & position, const glm::vec3 & target);
			virtual ~camera () { }

		private:
			void m_recalculate_view ();
	};

	class default_camera : public camera {
		private:
			glm::mat4x4 m_projection, m_projection_inv;
			float m_fov, m_near, m_far, m_aspect;

		public:
			float get_fov () const;
			float get_near () const;
			float get_far () const;
			float get_aspect () const;

			void set_fov (float fov);
			void set_near (float near);
			void set_far (float far);
			void set_aspect (float aspect);

			default_camera ();
			default_camera (const glm::vec3 & position, const glm::vec3 & target);

			virtual const glm::mat4x4 & get_projection_matrix () const override;
			virtual const glm::mat4x4 & get_projection_inverse () const override;
			virtual void video_mode_change (const video_mode_t & mode) override;

		private:
			void m_recalculate_projection ();
	};
}