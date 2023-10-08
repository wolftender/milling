#include "store.hpp"

#include <fstream>
#include <sstream>
#include <ios>
#include <iostream>

namespace mini {
	std::shared_ptr<shader_program> resource_store::m_load_shader(const std::string& vs_file, const std::string& ps_file) const {
		const std::string vs_source = m_read_file_content(vs_file);
		const std::string ps_source = m_read_file_content(ps_file);

		auto shader = std::make_shared<shader_program>(vs_source, ps_source);

		try {
			shader->compile();
		} catch (const shader_error& error) {
			std::cerr << error.what() << " log: " << std::endl << error.get_log() << std::endl;
			return nullptr;
		}

		return shader;
	}

	std::shared_ptr<shader_program> resource_store::m_load_shader(const std::string& vs_file, const std::string& ps_file, const std::string& gs_file) const {
		const std::string vs_source = m_read_file_content(vs_file);
		const std::string ps_source = m_read_file_content(ps_file);
		const std::string gs_source = m_read_file_content(gs_file);

		auto shader = std::make_shared<shader_program>(vs_source, ps_source);
		shader->set_geometry_source(gs_source);

		try {
			shader->compile();
		} catch (const shader_error& error) {
			std::cerr << error.what() << " log: " << std::endl << error.get_log() << std::endl;
			return nullptr;
		}

		return shader;
	}

	std::shared_ptr<shader_program> resource_store::m_load_shader(const std::string& vs_file, const std::string& ps_file, const std::string& tcs_file, const std::string& tes_file) const {
		const std::string vs_source = m_read_file_content(vs_file);
		const std::string ps_source = m_read_file_content(ps_file);
		const std::string tes_source = m_read_file_content(tes_file);
		const std::string tcs_source = m_read_file_content(tcs_file);

		auto shader = std::make_shared<shader_program>(vs_source, ps_source);
		shader->set_tesselation_source(tcs_source, tes_source);

		try {
			shader->compile();
		} catch (const shader_error& error) {
			std::cerr << error.what() << " log: " << std::endl << error.get_log() << std::endl;
			return nullptr;
		}

		return shader;
	}

	std::shared_ptr<shader_program> resource_store::m_load_shader(const std::string& vs_file, const std::string& ps_file, const std::string& tcs_file, const std::string& tes_file, const std::string& gs_file) const {
		const std::string vs_source = m_read_file_content(vs_file);
		const std::string ps_source = m_read_file_content(ps_file);
		const std::string tes_source = m_read_file_content(tes_file);
		const std::string tcs_source = m_read_file_content(tcs_file);
		const std::string gs_source = m_read_file_content(gs_file);

		auto shader = std::make_shared<shader_program>(vs_source, ps_source);

		shader->set_tesselation_source(tcs_source, tes_source);
		shader->set_geometry_source(gs_source);

		try {
			shader->compile();
		} catch (const shader_error& error) {
			std::cerr << error.what() << " log: " << std::endl << error.get_log() << std::endl;
			return nullptr;
		}

		return shader;
	}

	void resource_store::load_shader(
		const std::string& name, 
		const std::string& vs_file, 
		const std::string& ps_file) {

		m_add_shader(name, m_load_shader(vs_file, ps_file));
	}

	void resource_store::load_shader(
		const std::string& name, 
		const std::string& vs_file, 
		const std::string& ps_file, 
		const std::string& gs_file) {

		m_add_shader(name, m_load_shader(vs_file, ps_file, gs_file));
	}

	void resource_store::load_shader(
		const std::string& name, 
		const std::string& vs_file, 
		const std::string& ps_file, 
		const std::string& tcs_file, 
		const std::string& tes_file) {
		
		m_add_shader(name, m_load_shader(vs_file, ps_file, tcs_file, tes_file));
	}

	void resource_store::load_shader(
		const std::string& name, 
		const std::string& vs_file, 
		const std::string& ps_file, 
		const std::string& tcs_file, 
		const std::string& tes_file, 
		const std::string& gs_file) {

		m_add_shader(name, m_load_shader(vs_file, ps_file, tcs_file, tes_file, gs_file));
	}

	shader_handle_t resource_store::get_shader(const std::string& name) const {
		auto it = m_shaders.find(name);
		if (it != m_shaders.end()) {
			return it->second;
		}

		return nullptr;
	}

	texture_handle_t resource_store::get_texture(const std::string& name) const {
		auto it = m_textures.find(name);
		if (it != m_textures.end()) {
			return it->second;
		}

		return nullptr;
	}

	resource_store::resource_store() {}

	std::string resource_store::m_read_file_content(const std::string& path) const {
		std::ifstream stream(path);

		if (stream) {
			std::stringstream ss;
			ss << stream.rdbuf();

			return ss.str();
		}

		throw std::runtime_error("failed to read file " + path);
	}

	void resource_store::m_add_shader(const std::string& name, shader_handle_t shader) {
		m_shaders[name] = shader;
	}

	void resource_store::m_add_texture(const std::string& name, texture_handle_t texture) {
		m_textures[name] = texture;
	}
}