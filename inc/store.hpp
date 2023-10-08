#pragma once
#include <memory>
#include <unordered_map>

#include "shader.hpp"
#include "texture.hpp"

namespace mini {
	using shader_handle_t = std::shared_ptr<shader_program>;
	using texture_handle_t = std::shared_ptr<texture>;

	class resource_store final {
		private:
			std::unordered_map<std::string, shader_handle_t> m_shaders;
			std::unordered_map<std::string, texture_handle_t> m_textures;

		public:
			void load_shader(
				const std::string & name, 
				const std::string & vs_file, 
				const std::string & ps_file
			);

			void load_shader(
				const std::string & name, 
				const std::string & vs_file, 
				const std::string & ps_file, 
				const std::string & gs_file
			);

			void load_shader(
				const std::string & name, 
				const std::string & vs_file, 
				const std::string & ps_file, 
				const std::string & tcs_file, 
				const std::string & tes_file
			);

			void load_shader(
				const std::string & name, 
				const std::string & vs_file, 
				const std::string & ps_file, 
				const std::string & tcs_file, 
				const std::string & tes_file, 
				const std::string & gs_file
			);

			shader_handle_t get_shader(const std::string & name) const;
			texture_handle_t get_texture(const std::string & name) const;

			resource_store();
			~resource_store() = default;

			resource_store(const resource_store&) = delete;
			resource_store& operator= (const resource_store&) = delete;

		private:
			std::string m_read_file_content(const std::string& path) const;

			void m_add_shader(const std::string & name, shader_handle_t shader);
			void m_add_texture(const std::string & name, texture_handle_t texture);

			std::shared_ptr<shader_program> m_load_shader(
				const std::string& vs_file, 
				const std::string& ps_file
			) const;

			std::shared_ptr<shader_program> m_load_shader(
				const std::string& vs_file, 
				const std::string& ps_file, 
				const std::string& gs_file
			) const;

			std::shared_ptr<shader_program> m_load_shader(
				const std::string& vs_file, 
				const std::string& ps_file,
				const std::string& tcs_file, 
				const std::string& tes_file
			) const;

			std::shared_ptr<shader_program> m_load_shader(
				const std::string& vs_file, 
				const std::string& ps_file,
				const std::string& tcs_file, 
				const std::string& tes_file, 
				const std::string& gs_file
			) const;
	};
}