#include "curve.hpp"

namespace mini {
    curve::curve(std::shared_ptr<shader_program> line_shader) {
        m_line_shader = line_shader;

        m_vao = 0;
        m_position_buffer = 0;
        m_index_buffer = 0;
        m_ready = false;
        m_line_width = 2.0f;

        m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    curve::curve(
        std::shared_ptr<shader_program> line_shader,
        const std::vector<glm::vec3>& points) {

        m_line_shader = line_shader;

        m_vao = 0;
        m_position_buffer = 0;
        m_index_buffer = 0;
        m_ready = false;
        m_line_width = 2.0f;

        m_color = { 1.0f, 1.0f, 1.0f, 1.0f };

        m_points = points;
        m_rebuild_buffers();
    }

    float curve::get_line_width() const {
        return m_line_width;
    }

    const glm::vec4& curve::get_color() const {
        return m_color;
    }

    void curve::set_line_width(float width) {
        m_line_width = width;
    }

    void curve::set_color(const glm::vec4& color) {
        m_color = color;
    }

    void curve::append_position(const glm::vec3& position) {
        m_points.insert(m_points.end(), position);
        m_rebuild_buffers();
    }

    void curve::prepend_position(const glm::vec3& position) {
        m_points.insert(m_points.begin(), position);
        m_rebuild_buffers();
    }

    void curve::append_positions(const std::vector<glm::vec3>& positions) {
        m_points.insert(m_points.end(), positions.begin(), positions.end());
        m_rebuild_buffers();
    }

    void curve::prepend_positions(const std::vector<glm::vec3>& positions) {
        m_points.insert(m_points.begin(), positions.begin(), positions.end());
        m_rebuild_buffers();
    }

    void curve::erase_head() {
        if (m_points.size() == 0) {
            return;
        }

        m_points.erase(m_points.begin());
        m_rebuild_buffers();
    }

    void curve::clear_points() {
        m_points.clear();
        m_free_buffers();
    }

    void curve::erase_tail() {
        if (m_points.size() == 0) {
            return;
        }

        m_points.erase(m_points.end() - 1);
        m_rebuild_buffers();
    }

    void curve::render(app_context& context, const glm::mat4x4& world_matrix) const {
        if (!m_ready) {
            return;
        }

        const auto& view_matrix = context.get_view_matrix();
        const auto& proj_matrix = context.get_projection_matrix();

        const auto& video_mode = context.get_video_mode();

        glm::vec2 resolution = {
            static_cast<float> (video_mode.get_buffer_width()),
            static_cast<float> (video_mode.get_buffer_height())
        };

        glBindVertexArray(m_vao);

        m_line_shader->bind();
        m_line_shader->set_uniform("u_world", world_matrix);
        m_line_shader->set_uniform("u_view", view_matrix);
        m_line_shader->set_uniform("u_projection", proj_matrix);
        m_line_shader->set_uniform("u_resolution", resolution);
        m_line_shader->set_uniform("u_line_width", m_line_width);
        m_line_shader->set_uniform("u_color", m_color);

        glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    };

    void curve::m_rebuild_buffers() {
        constexpr GLuint a_position = 0;
        constexpr GLuint a_color = 1;

        if (m_ready) {
            m_free_buffers();
        }

        m_ready = false;

        if (m_points.size() == 0) {
            return;
        }

        m_positions.clear();
        m_positions.resize(m_points.size() * 3);

        for (int i = 0; i < m_points.size(); ++i) {
            m_positions[3 * i + 0] = m_points[i].x;
            m_positions[3 * i + 1] = m_points[i].y;
            m_positions[3 * i + 2] = m_points[i].z;
        }

        m_indices.clear();
        m_indices.reserve(m_points.size() * 2);

        for (uint32_t i = 0; i < m_points.size() - 1; ++i) {
            m_indices.push_back(i);
            m_indices.push_back(i + 1);
        }

        // allocate gpu buffers
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_position_buffer);
        glGenBuffers(1, &m_index_buffer);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), reinterpret_cast<void*> (m_positions.data()), GL_STATIC_DRAW);
        glVertexAttribPointer(a_position, 3, GL_FLOAT, false, sizeof(float) * 3, (void*)0);
        glEnableVertexAttribArray(a_position);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
        m_ready = true;
    }

    void curve::m_free_buffers() {
        m_ready = false;

        if (m_vao) {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }

        if (m_position_buffer) {
            glDeleteBuffers(1, &m_position_buffer);
            m_position_buffer = 0;
        }

        if (m_index_buffer) {
            glDeleteBuffers(1, &m_index_buffer);
            m_index_buffer = 0;
        }
    }
}
