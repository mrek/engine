/**
 * @file
 */

#pragma once

#include "VoxelVertex.h"
#include <vector>
#include <glm/vector_relational.hpp>

namespace voxel {

using VertexArray = std::vector<voxel::VoxelVertex>;
using IndexArray = std::vector<voxel::IndexType>;

/**
 * @brief A simple and general-purpose mesh class to represent the data returned by the surface extraction functions.
 *
 * @note You are only able to store vertex ranges from 0 to 255 here, due to the limited data type of the position in
 * the Vertex class.
 */
class Mesh {
public:
	Mesh(int vertices, int indices, bool mayGetResized = false);
	Mesh() : Mesh(128, 128, true) {}

	/**
	 * @brief Calculate the memory amount this mesh is using
	 */
	size_t size();

	/**
	 * @brief Transforms another mesh into this mesh class. The indices are offset.
	 * @return @c true if the given mesh is compatible to this mesh instance, @c false
	 * otherwise.
	 * @note Incompatible mesh means that the offsets don't match. This is needed due to the limited range of vertices
	 * due to the Vertex class position data type. Therefore we can merge meshes, but only if the offset is the same
	 * (as we can't exceed the 0-255 range).
	 */
	bool addMesh(const Mesh& mesh);

	size_t getNoOfVertices() const;
	const VoxelVertex& getVertex(IndexType index) const;
	const VoxelVertex* getRawVertexData() const;

	size_t getNoOfIndices() const;
	IndexType getIndex(IndexType index) const;
	const IndexType* getRawIndexData() const;

	const IndexArray& getIndexVector() const;
	const VertexArray& getVertexVector() const;
	IndexArray& getIndexVector();
	VertexArray& getVertexVector();

	const glm::ivec3& getOffset() const;
	void setOffset(const glm::ivec3& offset);

	IndexType addVertex(const VoxelVertex& vertex);
	void addTriangle(IndexType index0, IndexType index1, IndexType index2);

	void clear();
	bool isEmpty() const;
	void removeUnusedVertices();

	inline bool operator<(const Mesh& rhs) const {
		return glm::all(glm::lessThan(getOffset(), rhs.getOffset()));
	}
private:
	alignas(16) IndexArray _vecIndices;
	alignas(16) VertexArray _vecVertices;
	glm::ivec3 _offset { 0, 0, 0 };
	bool _mayGetResized;
};

}
