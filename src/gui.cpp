#include "gui.hpp"

namespace mini {
	namespace gui {
		void prefix_label (const std::string & label, float min_width) {
			float width = ImGui::CalcItemWidth ();

			if (width < min_width) {
				width = min_width;
			}

			float x = ImGui::GetCursorPosX ();
			ImGui::Text (label.c_str ());
			ImGui::SameLine ();
			ImGui::SetCursorPosX (x + width * 0.5f + ImGui::GetStyle ().ItemInnerSpacing.x);
			ImGui::SetNextItemWidth (-1);
		}

		bool vector_editor (const std::string & label, glm::vec2 & vector) {
			const std::string label_x = "##" + label + "_x";
			const std::string label_y = "##" + label + "_y";

			bool changed = false;

			if (ImGui::TreeNode (label.c_str ())) {
				gui::prefix_label ("X: ", 100.0f);
				changed = ImGui::InputFloat (label_x.c_str (), &vector[0]) || changed;

				gui::prefix_label ("Y: ", 100.0f);
				changed = ImGui::InputFloat (label_y.c_str (), &vector[1]) || changed;

				ImGui::TreePop ();
			}

			return changed;
		}

		bool vector_editor (const std::string & label, glm::vec3 & vector) {
			const std::string label_x = "##" + label + "_x";
			const std::string label_y = "##" + label + "_y";
			const std::string label_z = "##" + label + "_z";

			bool changed = false;

			if (ImGui::TreeNode (label.c_str ())) {
				gui::prefix_label ("X: ", 100.0f);
				changed = ImGui::InputFloat (label_x.c_str (), &vector[0]) || changed;

				gui::prefix_label ("Y: ", 100.0f);
				changed = ImGui::InputFloat (label_y.c_str (), &vector[1]) || changed;

				gui::prefix_label ("Z: ", 100.0f);
				changed = ImGui::InputFloat (label_z.c_str (), &vector[2]) || changed;

				ImGui::TreePop ();
			}

			return changed;
		}

		bool color_editor (const std::string & label, glm::vec4 & color) {
			float c[3] = { color.r, color.g, color.b };

			if (ImGui::ColorEdit3 (label.c_str (), c)) {
				color.r = c[0];
				color.g = c[1];
				color.b = c[2];
				color.a = 1.0f;

				return true;
			}

			return false;
		}
	}
}