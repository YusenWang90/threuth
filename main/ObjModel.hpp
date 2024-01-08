#pragma once

#include <string>

#include "glm.hpp"

struct MeshVertex
{
	MeshVertex() = default;
	MeshVertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty)
		: position{ x, y, z }, normal(nx, ny, nz), texcoord(tx, ty)
	{}

	bool operator==(const MeshVertex& other) const {
		return (other.position == position) &&
			(other.normal == normal) &&
			(other.texcoord == texcoord);
	}

	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 normal{ 0.0f, 0.0f, 0.0f };
	glm::vec2 texcoord{ 0.0f, 0.0f };
};

namespace std {
	template<> struct hash<MeshVertex> {
		size_t operator()(MeshVertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position)
				^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1)
				^ (hash<glm::vec2>()(vertex.texcoord) << 1);
		}
	};
}

struct ObjMesh
{
	void addVertex(const MeshVertex& vertex) { vertices.emplace_back(vertex); }
	void addIndex(uint32_t index) { indices.emplace_back(index); }

	const std::vector<MeshVertex>& getVertices() const { return vertices; }
	const std::vector<uint32_t>& getIndices() const { return indices; }

	uint32_t VAO;
	uint32_t VBO;
	uint32_t EBO;

	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
};

class ObjModel
{
public:
	bool load(const std::string& path);

	void createBuffers();

	void draw() const;

	ObjMesh mesh;
};