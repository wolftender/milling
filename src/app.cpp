#include "app.hpp"
#include "gui.hpp"

#include <iostream>
#include <variant>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
	
namespace mini {
	float application::get_cam_yaw() const {
		return m_cam_yaw;
	}

	float application::get_cam_pitch() const {
		return m_cam_pitch;
	}

	float application::get_cam_distance() const {
		return m_distance;
	}

	float application::get_time() const {
		return m_time;
	}

	bool application::is_viewport_focused() const {
		return m_viewport_focus;
	}

	bool application::is_mouse_in_viewport() const {
		return m_mouse_in_viewport;
	}

	const glm::vec3& application::get_cam_target() const {
		return m_camera_target;
	}

	void application::set_cam_target(const glm::vec3& target) {
		m_camera_target = target;
	}

	application::application() : 
		app_window(1200, 800, "milling application"),
		m_context(video_mode_t(1200, 800)) {

		m_cam_pitch = 0.0f;
		m_cam_yaw = 0.0f;
		m_distance = 10.0f;
		m_time = 0.0f;
		m_grid_spacing = 1.0f;
		m_grid_enabled = true;
		m_viewport_focus = false;
		m_mouse_in_viewport = false;
		m_last_vp_height = m_last_vp_width = 0;

		m_camera_target = { 0.0f, 0.0f, 0.0f };

		// load some basic shaders
		m_store.load_shader("basic", "shaders/vs_basic.glsl", "shaders/fs_basic.glsl");
		m_store.load_shader("grid_xz", "shaders/vs_grid.glsl", "shaders/fs_grid_xz.glsl");
		m_store.load_shader("billboard", "shaders/vs_billboard.glsl", "shaders/fs_billboard.glsl");
		m_store.load_shader("billboard_s", "shaders/vs_billboard_s.glsl", "shaders/fs_billboard.glsl");
		m_store.load_shader("millable", "shaders/vs_millable.glsl", "shaders/fs_millable.glsl");
		m_store.load_shader("millable_w", "shaders/vs_millable_w.glsl", "shaders/fs_millable_w.glsl");

		// objects
		m_grid_xz = std::make_shared<grid_object>(m_store.get_shader("grid_xz"));

		// millable block
		m_block = std::make_shared<millable_block>(
			m_store.get_shader("millable"), 
			m_store.get_shader("millable_w"), 
			1000, 
			1000);

		m_block->set_block_size({10.0f, 4.0f, 10.0f});
	}

	void application::t_integrate(float delta_time) {
		constexpr float pi2 = glm::pi<float>() * 2.0f;

		m_time = m_time + delta_time;
		m_handle_mouse();

		gui::clamp(m_cam_pitch, -1.56f, 1.56f);

		if (m_cam_yaw > pi2) {
			m_cam_yaw = m_cam_yaw - pi2;
		} else if (m_cam_yaw < -pi2) {
			m_cam_yaw = m_cam_yaw + pi2;
		}

		gui::clamp(m_distance, 1.0f, 20.0f);
		gui::clamp(m_grid_spacing, 0.05f, 10.0f);

		// setup camera for the scene
		glm::vec4 cam_pos = { 0.0f, 0.0f, -m_distance, 1.0f };
		glm::mat4x4 cam_rotation(1.0f);

		cam_rotation = glm::translate (cam_rotation, m_camera_target);
		cam_rotation = glm::rotate (cam_rotation, m_cam_yaw, { 0.0f, 1.0f, 0.0f });
		cam_rotation = glm::rotate (cam_rotation, m_cam_pitch, { 1.0f, 0.0f, 0.0f });

		cam_pos = cam_rotation * cam_pos;

		m_context.get_camera().set_position(cam_pos);
		m_context.get_camera().set_target(m_camera_target);

		m_cutter.update(delta_time, *m_block.get());

		app_window::t_integrate(delta_time);
	}

	void application::t_render() {
		if (m_grid_enabled) {
			m_grid_xz->set_spacing(m_grid_spacing);
			m_context.draw(m_grid_xz, glm::mat4x4(1.0f));
		}

		auto block_matrix = glm::mat4x4(1.0f);
		block_matrix = glm::translate(block_matrix, m_block->get_block_position());
		block_matrix = glm::scale(block_matrix, m_block->get_block_size());

		m_context.draw(m_block, block_matrix);
		m_context.display(false, true);

		m_draw_main_window();
		m_draw_viewport();
		m_draw_view_options();
		m_draw_scene_options();
	}

	void application::t_on_character(unsigned int code) {
		app_window::t_on_character(code);
	}

	void application::t_on_cursor_pos(double posx, double posy) {
		app_window::t_on_cursor_pos(posx, posy);
	}

	void application::t_on_mouse_button(int button, int action, int mods) {
		app_window::t_on_mouse_button(button, action, mods);
	}

	void application::t_on_key_event(int key, int scancode, int action, int mods) {
		if (action == GLFW_RELEASE && !ImGui::GetIO().WantCaptureKeyboard) {
			switch (key) {
				case GLFW_KEY_W:
					std::cout << "w key pressed" << std::endl;
					break;

				default:
					break;
			}
		}

		app_window::t_on_key_event(key, scancode, action, mods);
	}

	void application::t_on_scroll(double offset_x, double offset_y) {
		if (is_viewport_focused() && is_mouse_in_viewport()) {
			m_distance = m_distance - (static_cast<float> (offset_y) / 2.0f);
		}
	}

	void application::t_on_resize(int width, int height) {
		if (width != 0 && height != 0) {
			video_mode_t new_vm = m_context.get_video_mode();

			new_vm.set_viewport_width(width);
			new_vm.set_viewport_width(height);

			m_context.set_video_mode(new_vm);
		}

		app_window::t_on_resize(width, height);
	}

	void application::m_handle_mouse() {
		if (is_left_click() && m_viewport_focus) {
			const offset_t& last_pos = get_last_mouse_offset();
			const offset_t& curr_pos = get_mouse_offset();

			int d_yaw = curr_pos.x - last_pos.x;
			int d_pitch = curr_pos.y - last_pos.y;

			float f_d_yaw = static_cast<float> (d_yaw);
			float f_d_pitch = static_cast<float> (d_pitch);

			f_d_yaw = f_d_yaw * 15.0f / static_cast<float> (get_width());
			f_d_pitch = f_d_pitch * 15.0f / static_cast<float> (get_height());

			m_cam_yaw = m_cam_yaw - f_d_yaw;
			m_cam_pitch = m_cam_pitch + f_d_pitch;
		}
	}

	void application::m_draw_main_menu() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl + N", nullptr, true)) {
					std::cout << "czesc nowy" << std::endl;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Fajne Menu")) {
				if (ImGui::MenuItem("Hej", "Esc", nullptr, true)) {
					std::cout << "czesc" << std::endl;
				}

				ImGui::EndMenu();
			}
		}

		ImGui::EndMenuBar();
		ImGui::PopStyleVar(1);
	}

	void application::m_draw_main_window() {
		static bool first_time = true;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		float vp_width = static_cast<float> (get_width());
		float vp_height = static_cast<float> (get_height());

		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Dockspace", nullptr,
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoResize);

		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(vp_width, vp_height));

		ImGuiID dockspace_id = ImGui::GetID("g_dockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();

		if (first_time) {
			first_time = false;

			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(vp_width, vp_height));

			auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.25f, nullptr, &dockspace_id);
			auto dock_id_left_bottom = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.45f, nullptr, &dock_id_left);

			ImGui::DockBuilderDockWindow("Viewport", dockspace_id);
			ImGui::DockBuilderDockWindow("View Options", dock_id_left);
			ImGui::DockBuilderDockWindow("Scene Options", dock_id_left_bottom);

			ImGui::DockBuilderFinish(dockspace_id);
		}

		ImGui::PopStyleVar(3);
		m_draw_main_menu();

		ImGui::End();
	}

	void application::m_draw_viewport() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(320, 240));
		ImGui::Begin("Viewport", NULL);
		ImGui::SetWindowPos(ImVec2(30, 30), ImGuiCond_Once);
		ImGui::SetWindowSize(ImVec2(640, 480), ImGuiCond_Once);

		if (ImGui::IsWindowFocused()) {
			m_viewport_focus = true;
		} else {
			m_viewport_focus = false;
		}

		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		auto window_pos = ImGui::GetWindowPos();

		int width = static_cast<int> (max.x - min.x);
		int height = static_cast<int> (max.y - min.y);

		const offset_t& mouse_offset = get_mouse_offset();
		m_vp_mouse_offset.x = mouse_offset.x - static_cast<int> (min.x + window_pos.x);
		m_vp_mouse_offset.y = mouse_offset.y - static_cast<int> (min.y + window_pos.y);

		if (m_vp_mouse_offset.x < 0 || m_vp_mouse_offset.x > width || m_vp_mouse_offset.y < 0 || m_vp_mouse_offset.y > height) {
			m_mouse_in_viewport = false;
		} else {
			m_mouse_in_viewport = true;
		}

		// if mouse is out of viewport then pretend its at the border
		gui::clamp(m_vp_mouse_offset.x, 0, width);
		gui::clamp(m_vp_mouse_offset.y, 0, height);

		if ((width != m_last_vp_width || height != m_last_vp_height) && width > 8 && height > 8) {
			video_mode_t mode(width, height);

			m_context.set_video_mode(mode);

			m_last_vp_width = width;
			m_last_vp_height = height;
		}

		ImGui::ImageButton(reinterpret_cast<ImTextureID> (m_context.get_front_buffer()), ImVec2(width, height), ImVec2(0, 0), ImVec2(1, 1), 0);

		ImGui::End();
		ImGui::PopStyleVar(1);
	}

	void application::m_draw_view_options() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(270, 450));
		ImGui::Begin("View Options", NULL);
		ImGui::SetWindowPos(ImVec2(30, 30), ImGuiCond_Once);
		ImGui::SetWindowSize(ImVec2(270, 450), ImGuiCond_Once);

		// render controls
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
			gui::prefix_label("Cam. Pitch: ", 250.0f);
			ImGui::InputFloat("##pitch", &m_cam_pitch);

			gui::prefix_label("Cam. Yaw: ", 250.0f);
			ImGui::InputFloat("##yaw", &m_cam_yaw);

			gui::prefix_label("Cam. Dist: ", 250.0f);
			ImGui::InputFloat("##distance", &m_distance);

			gui::vector_editor("Cam. Target", m_camera_target);
			ImGui::NewLine();
		}

		ImGui::End();
		ImGui::PopStyleVar(1);
	}

	void application::m_draw_scene_options() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(270, 450));
		ImGui::Begin("Scene Options", NULL);
		ImGui::SetWindowPos(ImVec2(30, 30), ImGuiCond_Once);
		ImGui::SetWindowSize(ImVec2(270, 450), ImGuiCond_Once);

		// render controls
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
			gui::prefix_label("Cam. Pitch: ", 250.0f);
			ImGui::InputFloat("##pitch", &m_cam_pitch);

			gui::prefix_label("Cam. Yaw: ", 250.0f);
			ImGui::InputFloat("##yaw", &m_cam_yaw);

			gui::prefix_label("Cam. Dist: ", 250.0f);
			ImGui::InputFloat("##distance", &m_distance);

			gui::vector_editor("Cam. Target", m_camera_target);
			ImGui::NewLine();
		}

		ImGui::End();
		ImGui::PopStyleVar(1);
	}
}