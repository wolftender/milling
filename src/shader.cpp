#include "shader.hpp"

#include <cstring>
#include <glm/gtc/type_ptr.hpp>

namespace mini {
	const std::string & shader_error::get_log () const {
		return m_log;
	}

	const shader_error_type_t & shader_error::get_type () const {
		return m_type;
	}

	shader_error::shader_error (shader_error_type_t type, const std::string & message) :
		std::runtime_error (message.c_str ()),
		m_type (type) { }

	shader_error::shader_error (shader_error_type_t type, const std::string & message, const std::string & log) :
		std::runtime_error (message.c_str ()),
		m_type (type),
		m_log (log) { }

	void shader_program::set_vertex_source (const std::string & source) {
		if (!m_is_ready) {
			m_vs_source = source;
		}
	}

	void shader_program::set_fragment_source (const std::string & source) {
		if (!m_is_ready) {
			m_ps_source = source;
		}
	}

	void shader_program::set_geometry_source (const std::string & source) {
		if (!m_is_ready) {
			m_gs_source = source;
		}
	}

	void shader_program::set_tesselation_source (const std::string & tcs, const std::string & tes) {
		if (!m_is_ready) {
			m_tcs_source = tcs;
			m_tes_source = tes;
		}
	}

	bool shader_program::compile () {
		// compile and link
		GLuint vs = 0;
		GLuint ps = 0;
		GLuint gs = 0;
		GLuint tcs = 0;
		GLuint tes = 0;

		// theese will throw exceptions if any fails
		try {
			m_try_compile (GL_VERTEX_SHADER, m_vs_source, &vs);
			m_try_compile (GL_FRAGMENT_SHADER, m_ps_source, &ps);

			if (m_gs_source.size () > 0) {
				m_has_geometry = true;
				m_try_compile (GL_GEOMETRY_SHADER, m_gs_source, &gs);
			}

			if (m_tcs_source.size () > 0 && m_tes_source.size () > 0) {
				m_has_tesselation = true;
				m_try_compile (GL_TESS_CONTROL_SHADER, m_tcs_source, &tcs);
				m_try_compile (GL_TESS_EVALUATION_SHADER, m_tes_source, &tes);
			}
		} catch (const std::exception & e) {
			if (vs) {
				glDeleteShader (vs);
			}

			if (ps) {
				glDeleteShader (ps);
			}

			if (gs) {
				glDeleteShader (gs);
			}

			if (tcs) {
				glDeleteShader (tcs);
			}

			if (tes) {
				glDeleteShader (tes);
			}

			// propagate the error
			throw e;
		}

		m_ps = ps;
		m_vs = vs;
		m_gs = gs;
		m_tcs = tcs;
		m_tes = tes;

		const GLuint program_object = glCreateProgram ();

		if (program_object == 0) {
			throw std::runtime_error ("failed to create shader program");
		}

		m_program = program_object;
		glAttachShader (m_program, m_vs);
		glAttachShader (m_program, m_ps);

		if (m_has_geometry) {
			glAttachShader (m_program, m_gs);
		}

		if (m_has_tesselation) {
			glAttachShader (m_program, m_tes);
			glAttachShader (m_program, m_tcs);
		}

		m_try_link ();

		// linker success, delete shaders as they are no longer needed
		glDeleteShader (m_vs);
		glDeleteShader (m_ps);

		if (m_has_geometry) {
			glDeleteShader (m_gs);
		}

		if (m_has_tesselation) {
			glDeleteShader (m_tcs);
			glDeleteShader (m_tes);
		}

		m_vs = m_ps = m_gs = m_tes = m_tcs = 0;
		return true;
	}

	bool shader_program::is_ready () const {
		return m_is_ready;
	}

	shader_program::shader_program () {
		m_is_ready = false;
		m_has_geometry = false;
		m_has_tesselation = false;

		m_program = m_ps = m_vs = m_gs = m_tcs = m_tes = 0;
	}

	shader_program::shader_program (const std::string & vs_source, const std::string & ps_source) {
		m_is_ready = false;
		m_has_geometry = false;
		m_has_tesselation = false;

		m_program = m_ps = m_vs = m_gs = m_tcs = m_tes = 0;

		set_vertex_source (vs_source);
		set_fragment_source (ps_source);
	}

	shader_program::shader_program (const shader_program & shader) {
		m_is_ready = false;
		m_has_geometry = false;
		m_has_tesselation = false;

		m_program = m_ps = m_vs = m_gs = m_tcs = m_tes = 0;

		m_vs_source = shader.m_vs_source;
		m_ps_source = shader.m_ps_source;

		compile ();
	}

	shader_program & shader_program::operator= (const shader_program & shader) {
		m_is_ready = false;
		m_has_geometry = false;
		m_has_tesselation = false;

		m_program = m_ps = m_vs = 0;

		m_vs_source = shader.m_vs_source;
		m_ps_source = shader.m_ps_source;

		if (shader.m_has_geometry) {
			m_has_geometry = true;
			m_gs_source = shader.m_gs_source;
		}

		if (shader.m_has_tesselation) {
			m_has_tesselation = true;
			m_tes_source = shader.m_tes_source;
			m_tcs_source = shader.m_tcs_source;
		}

		compile ();

		return (*this);
	}

	shader_program::~shader_program () {
		if (m_program) {
			glDeleteProgram (m_program);
		}

		if (m_vs) {
			glDeleteShader (m_vs);
		}

		if (m_ps) {
			glDeleteShader (m_ps);
		}

		if (m_gs) {
			glDeleteShader (m_gs);
		}

		if (m_tcs) {
			glDeleteShader (m_tcs);
		}

		if (m_tes) {
			glDeleteShader (m_tes);
		}
	}

	void shader_program::bind () const {
		if (m_is_ready) {
			glUseProgram (m_program);
		}
	}

	GLuint shader_program::get_program_handle () const {
		return m_program;
	}

	int shader_program::get_uniform_location (const std::string & name) {
		if (!m_is_ready) {
			throw std::runtime_error ("program is not linked: cannot get uniform");
		}

		const int location = glGetUniformLocation (m_program, name.c_str ());
		return location;
	}

	void shader_program::set_uniform_sampler (const std::string & name, const GLint value) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniform1i (location, value);
		}
	}

	void shader_program::set_uniform_int (const std::string & name, const GLint value) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniform1i (location, value);
		}
	}

	void shader_program::set_uniform_uint (const std::string & name, const GLuint value) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniform1ui (location, value);
		}
	}

	void shader_program::set_uniform (const std::string & name, const float value) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniform1f (location, value);
		}
	}

	void shader_program::set_uniform (const std::string & name, const glm::vec2 & vector) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniform2fv (location, 1, glm::value_ptr (vector));
		}
	}

	void shader_program::set_uniform (const std::string & name, const glm::vec3 & vector) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniform3fv (location, 1, glm::value_ptr (vector));
		}
	}

	void shader_program::set_uniform (const std::string & name, const glm::vec4 & vector) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniform4fv (location, 1, glm::value_ptr (vector));
		}
	}

	void shader_program::set_uniform (const std::string & name, const glm::mat3x3 & matrix) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniformMatrix3fv (location, 1, GL_FALSE, glm::value_ptr (matrix));
		}
	}

	void shader_program::set_uniform (const std::string & name, const glm::mat4x4 & matrix) {
		const int location = get_uniform_location (name);

		if (location >= 0) {
			glUniformMatrix4fv (location, 1, GL_FALSE, glm::value_ptr (matrix));
		}
	}

	bool shader_program::m_try_compile (GLenum shader_type, const std::string & source, GLuint * out_shader) {
		const GLuint shader_object = glCreateShader (shader_type);

		// error has occured, invalid enum provided
		if (shader_object == 0) {
			throw std::runtime_error ("invalid shader enum was provided as a function argument");
		}

		const char * ptrSourceBuffer = source.c_str ();
		glShaderSource (shader_object, 1, &ptrSourceBuffer, NULL);

		// compile shader
		glCompileShader (shader_object);

		// error checking
		GLint compile_status = 0;
		glGetShaderiv (shader_object, GL_COMPILE_STATUS, &compile_status);

		if (compile_status != GL_TRUE) {
			char compile_log_buffer[4096];
			int compile_log_len = 0;

			// zero out the log buffer
			memset (compile_log_buffer, 0, 4096);

			// make a log and throw an error
			glGetShaderInfoLog (shader_object, 4096, &compile_log_len, compile_log_buffer);

			// clean up and throw
			glDeleteShader (shader_object);
			throw shader_error (shader_error_type_t::compile_shader, "failed to compile shader", std::string (compile_log_buffer));

			return false;
		}

		(*out_shader) = shader_object;
		return true;
	}

	bool shader_program::m_try_link () {
		if (m_is_ready) {
			throw std::runtime_error ("this shader was already linked and compiled");
		}

		glLinkProgram (m_program);

		GLint linkStatus = 0;
		glGetProgramiv (m_program, GL_LINK_STATUS, &linkStatus);

		if (linkStatus != GL_TRUE) {
			char link_log_buffer[4096];
			int link_log_len = 0;

			// zero out the log buffer
			memset (link_log_buffer, 0, 4096);

			// make a log and throw an error
			glGetProgramInfoLog (m_program, 4096, &link_log_len, link_log_buffer);

			// clean up and throw
			glDeleteProgram (m_program);

			throw shader_error (shader_error_type_t::link_program, "failed to link shader", std::string (link_log_buffer));
		}

		m_is_ready = true;
		return true;
	}
}