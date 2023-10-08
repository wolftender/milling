#include "texture.hpp"

#include <lodepng.h>

#include <ios>
#include <fstream>
#include <stdexcept>

namespace mini {
	uint32_t texture::get_width () const {
		return m_width;
	}

	uint32_t texture::get_height () const {
		return m_height;
	}

	uint32_t texture::get_mipmap_levels () const {
		return m_mipmap_levels;
	}

	GLenum texture::get_min_filter () const {
		return m_min_filter;
	}

	GLenum texture::get_mag_filter () const {
		return m_mag_filter;
	}

	GLenum texture::get_format () const {
		return m_format;
	}

	texture::texture (uint32_t width, uint32_t height, unsigned char * data) {
		m_width = width;
		m_height = height;

		const uint64_t size = width * height;

		m_data.reserve (size);
		m_data.insert (m_data.begin (), data, data + size);

		m_mipmap_levels = 0;
		m_min_filter = GL_LINEAR;
		m_mag_filter = GL_LINEAR;
		m_format = GL_RGB;

		m_initialize ();
	}

	texture::texture (uint32_t width, uint32_t height, unsigned char * data, GLenum format, 
		unsigned int mipmap_levels, GLenum min_filter, GLenum mag_filter) {

		m_width = width;
		m_height = height;

		uint64_t size = width * height;

		switch (format) {
			case GL_RGB:
				size = size * 3;
				break;

			case GL_RGBA:
				size = size * 4;
				break;
		}

		m_data.reserve (size);
		m_data.insert (m_data.begin (), data, data + size);

		m_mipmap_levels = mipmap_levels;
		m_min_filter = min_filter;
		m_mag_filter = mag_filter;
		m_format = format;

		m_initialize ();
	}

	texture::~texture () {
		glDeleteTextures (1, &m_texture);
	}

	void texture::bind (GLenum slot) const {
		glActiveTexture (slot);
		glBindTexture (GL_TEXTURE_2D, m_texture);
	}

	void texture::m_initialize () {
		glGenTextures (1, &m_texture);

		// load the bytes into the texture
		glBindTexture (GL_TEXTURE_2D, m_texture);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);

		glTexImage2D (GL_TEXTURE_2D, m_mipmap_levels, m_format, m_width, m_height, 0, m_format, GL_UNSIGNED_BYTE, m_data.data ());
		glBindTexture (GL_TEXTURE_2D, static_cast<GLuint>(NULL));
	}

	std::shared_ptr<texture> texture::load_from_file (const std::string & file) {
		std::fstream stream (file, std::ios::binary | std::ios::in);

		if (!stream) {
			throw std::runtime_error ("cannot open file: " + file);
		}

		stream.seekg (0, std::ios::end);
		uint64_t size = stream.tellg ();
		stream.seekg (0, std::ios::beg);

		std::vector<uint8_t> buffer;
		buffer.resize (size);

		stream.read (reinterpret_cast<char *> (buffer.data ()), size);
		stream.close ();

		std::vector<unsigned char> data;
		unsigned int width, height;

		unsigned int error = lodepng::decode (data, width, height, buffer);

		if (error != 0) { // failed to load image
			throw std::runtime_error ("failed to load texture: " + std::string (lodepng_error_text (error)));
		}

		return std::make_shared<texture> (width, height, data.data (), GL_RGBA);
	}
}