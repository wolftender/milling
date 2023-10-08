#pragma once
#include <string>
#include <stdexcept>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace mini {
	enum class shader_error_type_t {
		compile_shader,
		link_program
	};

	class shader_error : public std::runtime_error {
		private:
			shader_error_type_t m_type;
			std::string m_log;

		public:
			const std::string & get_log () const;
			const shader_error_type_t & get_type () const;

			shader_error (shader_error_type_t type, const std::string & message);
			shader_error (shader_error_type_t type, const std::string & message, const std::string & log);
	};

	/// <summary>
	/// This is a simple wrapper class around a shader program.
	/// Probably it can be expanded with geometry shader in the future.
	/// </summary>
	class shader_program {
		private:
			std::string m_vs_source, m_ps_source, m_gs_source, m_tcs_source, m_tes_source;
			GLuint m_program, m_ps, m_vs, m_gs, m_tcs, m_tes;
			bool m_is_ready, m_has_geometry, m_has_tesselation;

		public:
			void set_vertex_source (const std::string & source);
			void set_fragment_source (const std::string & source);
			void set_geometry_source (const std::string & source);
			void set_tesselation_source (const std::string & tcs, const std::string & tes);
			bool compile ();

			bool is_ready () const;
			
			shader_program ();
			shader_program (const std::string & vs_source, const std::string & ps_source);

			shader_program (const shader_program & shader);
			shader_program & operator= (const shader_program & shader);

			~shader_program ();

			void bind () const;
			GLuint get_program_handle () const;

			int get_uniform_location (const std::string & name);

			void set_uniform_sampler (const std::string & name, const GLint value);
			void set_uniform_int (const std::string & name, const GLint value);
			void set_uniform_uint (const std::string & name, const GLuint value);
			void set_uniform (const std::string & name, const float value);
			void set_uniform (const std::string & name, const glm::vec2 & vector);
			void set_uniform (const std::string & name, const glm::vec3 & vector);
			void set_uniform (const std::string & name, const glm::vec4 & vector);
			void set_uniform (const std::string & name, const glm::mat3x3 & matrix);
			void set_uniform (const std::string & name, const glm::mat4x4 & matrix);

		private:
			bool m_try_compile (GLenum shader_type, const std::string & source, GLuint * out_shader);
			bool m_try_link ();
	};
}