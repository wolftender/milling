#pragma once
#include <memory>
#include <string>
#include <chrono>
#include <unordered_set>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace mini {
	struct offset_t { int x, y; };
	struct glfw_window_deleter_t {
		public:
			void operator() (GLFWwindow * window);
	};

	using glfw_window_ptr_t = std::unique_ptr<GLFWwindow, glfw_window_deleter_t>;

	/// <summary>
	/// This class represents a single GLFW window. I think there only can be one window anyways
	/// because of how context is assigned? Also this class probably has to be instantiated
	/// inside main thread for portability.
	/// </summary>
	class app_window {
		// properties
		private:
			// window properties
			glfw_window_ptr_t m_window;
			uint32_t m_width, m_height;
			std::string m_title;

			// delta time calculations
			std::chrono::steady_clock::time_point m_last_frame;

			// user input stuff
			offset_t m_last_mouse, m_mouse;
			bool m_left_click, m_right_click, m_middle_click;
			std::unordered_set<int> m_pressed_keys;

		// methods
		private:
			static void s_on_key_event (GLFWwindow * window, int key, int scancode, int action, int mods);
			static void s_on_character (GLFWwindow * window, unsigned int code);
			static void s_on_cursor_pos (GLFWwindow * window, double posx, double posy);
			static void s_on_mouse_button (GLFWwindow * window, int button, int action, int mods);
			static void s_on_resize (GLFWwindow * window, int width, int height);
			static void s_on_scroll (GLFWwindow * window, double offset_x, double offset_y);

			void m_setup_imgui ();
			void m_setup_imgui_style ();

		protected:
			virtual void t_integrate (float delta_time);
			virtual void t_render ();

			virtual void t_on_key_event (int key, int scancode, int action, int mods);
			virtual void t_on_character (unsigned int code);
			virtual void t_on_cursor_pos (double posx, double posy);
			virtual void t_on_mouse_button (int button, int action, int mods);
			virtual void t_on_resize (int width, int height);
			virtual void t_on_scroll (double offset_x, double offset_y);

		public:
			uint32_t get_width () const;
			uint32_t get_height () const;
			const std::string & get_title () const;

			const offset_t & get_mouse_offset () const;
			const offset_t & get_last_mouse_offset () const;

			bool is_left_click () const;
			bool is_right_click () const;
			bool is_middle_click () const;
			bool is_key_down (int key) const;

			void set_width (uint32_t width);
			void set_height (uint32_t height);
			void set_size (uint32_t width, uint32_t height);
			void set_title (const std::string & title);

			app_window (uint32_t width, uint32_t height, const std::string & title);
			virtual ~app_window ();

			app_window (const app_window &) = delete;
			app_window & operator= (const app_window &) = delete;

			void message_loop ();
	};
}