#include <fstream>
#include <array>
#include <map>

#include <glm/gtc/constants.hpp>

#include "mesh.hpp"

namespace mini {
	constexpr const uint32_t LAYOUT_POSITION = 0;
	constexpr const uint32_t LAYOUT_NORMAL = 1;

	const std::vector<float>& triangle_mesh::get_positions() const {
		return m_positions;
	}

	const std::vector<float>& triangle_mesh::get_normals() const {
		return m_normals;
	}

	const std::vector<uint32_t>& triangle_mesh::get_indices() const {
		return m_indices;
	}

	triangle_mesh::triangle_mesh(const std::vector<float>& positions, const std::vector<float>& normals, const std::vector<uint32_t>& indices) {
		m_positions = positions;
		m_normals = normals;
		m_indices = indices;

		m_array_object = m_position_buffer = m_normal_buffer = m_index_buffer = 0;
		m_initialize();
	}

	triangle_mesh::~triangle_mesh() {
		m_destroy();
	}

	triangle_mesh::triangle_mesh(const triangle_mesh& mesh) {
		m_positions = mesh.m_positions;
		m_normals = mesh.m_normals;
		m_indices = mesh.m_indices;

		m_array_object = m_position_buffer = m_normal_buffer = m_index_buffer = 0;
		m_initialize();
	}

	triangle_mesh& triangle_mesh::operator=(const triangle_mesh& mesh) {
		m_destroy();

		m_positions = mesh.m_positions;
		m_normals = mesh.m_normals;
		m_indices = mesh.m_indices;

		m_array_object = m_position_buffer = m_normal_buffer = m_index_buffer = 0;
		m_initialize();

		return (*this);
	}

	void triangle_mesh::draw() {
		glBindVertexArray(m_array_object);
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
	}

	void triangle_mesh::m_initialize() {
		glGenVertexArrays(1, &m_array_object);
		glGenBuffers(1, &m_position_buffer);
		glGenBuffers(1, &m_normal_buffer);
		glGenBuffers(1, &m_index_buffer);

		glBindVertexArray(m_array_object);

		// initialize buffers with data
		glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), m_positions.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(LAYOUT_POSITION, 3, GL_FLOAT, false, sizeof(float) * 3, nullptr);
		glEnableVertexAttribArray(LAYOUT_POSITION);

		glBindBuffer(GL_ARRAY_BUFFER, m_normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_normals.size(), m_normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(LAYOUT_NORMAL, 3, GL_FLOAT, false, sizeof(float) * 3, nullptr);
		glEnableVertexAttribArray(LAYOUT_NORMAL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void triangle_mesh::m_destroy() {
		if (m_array_object != 0) {
			glDeleteVertexArrays(1, &m_array_object);
		}

		if (m_position_buffer != 0) {
			glDeleteBuffers(1, &m_position_buffer);
		}

		if (m_normal_buffer != 0) {
			glDeleteBuffers(1, &m_normal_buffer);
		}

		if (m_index_buffer != 0) {
			glDeleteBuffers(1, &m_index_buffer);
		}

		m_array_object = m_position_buffer = m_normal_buffer = m_index_buffer = 0;
	}

	template<typename T> inline T min(T a, T b) {
		return (a <= b) ? a : b;
	}

	std::unique_ptr<triangle_mesh> triangle_mesh::make_plane_mesh() {
		std::vector<float> plane_positions = {
			 1, -1,  0,
			 1,  1,  0,
			-1,  1,  0,
			-1, -1,  0,
			-1, -1,  0,
			-1,  1,  0,
			 1,  1,  0,
			 1, -1,  0
		};

		std::vector<float> plane_normals = {
			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1,
			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1
		};

		std::vector<uint32_t> plane_indices = {
			2, 1, 0, 3, 2, 0, 6, 5, 4, 7, 6, 4
		};

		return std::make_unique<triangle_mesh>(plane_positions, plane_normals, plane_indices);
	}

	std::unique_ptr<triangle_mesh> triangle_mesh::make_plane_mesh_front() {
		std::vector<float> plane_positions = {
			 1, -1,  0,
			 1,  1,  0,
			-1,  1,  0,
			-1, -1,  0,
		};

		std::vector<float> plane_normals = {
			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1
		};

		std::vector<uint32_t> plane_indices = {
			2, 1, 0, 3, 2, 0
		};

		return std::make_unique<triangle_mesh>(plane_positions, plane_normals, plane_indices);
	}

	std::unique_ptr<triangle_mesh> triangle_mesh::make_plane_mesh_back() {
		std::vector<float> plane_positions = {
			-1, -1,  0,
			-1,  1,  0,
			 1,  1,  0,
			 1, -1,  0
		};

		std::vector<float> plane_normals = {
			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1
		};

		std::vector<uint32_t> plane_indices = {
			2, 1, 0, 3, 2, 0
		};

		return std::make_unique<triangle_mesh>(plane_positions, plane_normals, plane_indices);
	}

	std::unique_ptr<triangle_mesh> triangle_mesh::make_cube_mesh() {
		std::vector<float> cube_positions = {
			 1,  1, -1,
			-1,  1, -1,
			-1,  1,  1,
			 1,  1,  1,
			 1, -1,  1,
			 1,  1,  1,
			-1,  1,  1,
			-1, -1,  1,
			-1, -1,  1,
			-1,  1,  1,
			-1,  1, -1,
			-1, -1, -1,
			-1, -1, -1,
			 1, -1, -1,
			 1, -1,  1,
			-1, -1,  1,
			 1, -1, -1,
			 1,  1, -1,
			 1,  1,  1,
			 1, -1,  1,
			-1, -1, -1,
			-1,  1, -1,
			 1,  1, -1,
			 1, -1, -1
		};

		std::vector<float> cube_normals = {
			 0,  1,  0,
			 0,  1,  0,
			 0,  1,  0,
			 0,  1,  0,
			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1,
			-1,  0,  0,
			-1,  0,  0,
			-1,  0,  0,
			-1,  0,  0,
			 0, -1,  0,
			 0, -1,  0,
			 0, -1,  0,
			 0, -1,  0,
			 1,  0,  0,
			 1,  0,  0,
			 1,  0,  0,
			 1,  0,  0,
			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1
		};

		std::vector<uint32_t> cube_indices = {
			0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23
		};

		return std::make_unique<triangle_mesh>(cube_positions, cube_normals, cube_indices);
	}

	std::unique_ptr<triangle_mesh> triangle_mesh::make_cylinder(float radius, float height, int latitudes, int longitudes) {
		std::vector<float> positions, normals;
		std::vector<uint32_t> indices;

		float vx, vy, vz, nx, ny, nz;
		float radiusInv = 1 / radius;

		float dLat = height / latitudes;
		float dLong = 2.0f * glm::pi<float>() / longitudes;

		float latHeight, longAng;

		for (uint32_t i = 0; i <= latitudes; ++i) {
			latHeight = i * dLat;

			if (i == 0) {
				latHeight += dLat;
			} else if (i == latitudes) {
				latHeight -= dLat;
			}

			float xy = (i == 0 || i == latitudes) ? 0 : radius;
			float z = latHeight;

			for (uint32_t j = 0; j <= longitudes; ++j) {
				longAng = j * dLong;
				vx = xy * glm::cos(longAng);
				vy = xy * glm::sin(longAng);
				vz = z;

				nx = vx * radiusInv;
				ny = vy * radiusInv;
				nz = 0.0f;

				positions.insert(positions.end(), { vx, vy, vz });
				normals.insert(normals.end(), { nx, ny, nz });
			}
		}

		uint32_t k1, k2;
		for (uint32_t i = 0; i < latitudes; ++i) {
			k1 = i * (longitudes + 1);
			k2 = k1 + longitudes + 1;

			for (uint32_t j = 0; j < longitudes; ++j, ++k1, ++k2) {
				if (i != 0) {
					indices.insert(indices.end(), { k1 + 1, k2, k1 });
				}

				if (i != (latitudes - 1)) {
					indices.insert(indices.end(), { k2 + 1, k2, k1 + 1 });
				}
			}
		}

		return std::make_unique<triangle_mesh>(positions, normals, indices);
	}
}
