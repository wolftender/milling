#pragma once
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "shader.hpp"
#include "context.hpp"

namespace mini {
    class curve : public graphics_object {
        private:
            std::vector<glm::vec3> m_points;
            std::shared_ptr<shader_program> m_line_shader;

            std::vector<float> m_positions;
            std::vector<uint32_t> m_indices;
            GLuint m_vao, m_position_buffer, m_index_buffer;

            bool m_ready;

            glm::vec4 m_color;
            float m_line_width;

        public:
            curve(
                std::shared_ptr<shader_program> line_shader
            );

            curve(
                std::shared_ptr<shader_program> line_shader,
                const std::vector<glm::vec3>& points
            );

            curve(const curve&) = delete;
            curve& operator=(const curve&) = delete;

            float get_line_width() const;
            const glm::vec4& get_color() const;

            void set_line_width(float width);
            void set_color(const glm::vec4& color);

            void append_position(const glm::vec3& position);
            void prepend_position(const glm::vec3& position);

            void append_positions(const std::vector<glm::vec3>& positions);
            void prepend_positions(const std::vector<glm::vec3>& positions);

            void erase_tail();
            void erase_head();

            void clear_points();

            virtual void render(app_context& context, const glm::mat4x4& world_matrix) const override;

        private:
            void m_rebuild_buffers();
            void m_free_buffers();
    };
}