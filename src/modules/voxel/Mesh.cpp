/**
 * @file
 */

#include "Mesh.h"
#include "CubicSurfaceExtractor.h"
#include "core/Common.h"
#include "core/Trace.h"
#include "core/Assert.h"

namespace voxel {

Mesh::Mesh(int vertices, int indices, bool mayGetResized) : _mayGetResized(mayGetResized) {
	if (vertices > 0) {
		_vecVertices.reserve(vertices);
	}
	if (indices > 0) {
		_vecIndices.reserve(indices);
	}
}

const std::vector<IndexType>& Mesh::getIndexVector() const {
	return _vecIndices;
}

const std::vector<VoxelVertex>& Mesh::getVertexVector() const {
	return _vecVertices;
}

std::vector<IndexType>& Mesh::getIndexVector() {
	return _vecIndices;
}

std::vector<VoxelVertex>& Mesh::getVertexVector() {
	return _vecVertices;
}

size_t Mesh::getNoOfVertices() const {
	return _vecVertices.size();
}

const VoxelVertex& Mesh::getVertex(IndexType index) const {
	return _vecVertices[index];
}

const VoxelVertex* Mesh::getRawVertexData() const {
	return _vecVertices.data();
}

size_t Mesh::getNoOfIndices() const {
	return _vecIndices.size();
}

IndexType Mesh::getIndex(IndexType index) const {
	return _vecIndices[index];
}

const IndexType* Mesh::getRawIndexData() const {
	return _vecIndices.data();
}

const glm::ivec3& Mesh::getOffset() const {
	return _offset;
}

void Mesh::setOffset(const glm::ivec3& offset) {
	_offset = offset;
}

void Mesh::clear() {
	_vecVertices.clear();
	_vecIndices.clear();
	_offset = glm::ivec3(0);
}

bool Mesh::isEmpty() const {
	return getNoOfVertices() == 0 || getNoOfIndices() == 0;
}

void Mesh::addTriangle(IndexType index0, IndexType index1, IndexType index2) {
	//Make sure the specified indices correspond to valid vertices.
	core_assert_msg(index0 < _vecVertices.size(), "Index points at an invalid vertex.");
	core_assert_msg(index1 < _vecVertices.size(), "Index points at an invalid vertex.");
	core_assert_msg(index2 < _vecVertices.size(), "Index points at an invalid vertex.");
	if (!_mayGetResized) {
		core_assert_msg(_vecIndices.size() + 3 < _vecIndices.capacity(), "addTriangle() call exceeds the capacity of the indices vector and will trigger a realloc (%i vs %i)", (int)_vecIndices.size(), (int)_vecIndices.capacity());
	}

	_vecIndices.push_back(index0);
	_vecIndices.push_back(index1);
	_vecIndices.push_back(index2);
}

IndexType Mesh::addVertex(const VoxelVertex& vertex) {
	// We should not add more vertices than our chosen index type will let us index.
	core_assert_msg(_vecVertices.size() < (std::numeric_limits<IndexType>::max)(), "Mesh has more vertices that the chosen index type allows.");
	if (!_mayGetResized) {
		core_assert_msg(_vecVertices.size() + 1 < _vecVertices.capacity(), "addVertex() call exceeds the capacity of the vertices vector and will trigger a realloc (%i vs %i)", (int)_vecVertices.size(), (int)_vecVertices.capacity());
	}

	_vecVertices.push_back(vertex);
	return (IndexType)_vecVertices.size() - 1;
}

size_t Mesh::size() {
	constexpr size_t classSize = sizeof(*this);
	const size_t indicesSize = _vecIndices.size() * sizeof(IndexType);
	const size_t verticesSize = _vecVertices.size() * sizeof(VoxelVertex);
	const size_t contentSize = indicesSize + verticesSize;
	return classSize + contentSize;
}

bool Mesh::addMesh(const Mesh& mesh) {
	if (mesh.getOffset() != getOffset()) {
		return false;
	}
	const IndexType* indices = mesh.getRawIndexData();
	const VoxelVertex* vertices = mesh.getRawVertexData();
	const size_t nIndices = mesh.getNoOfIndices();
	const size_t nVertices = mesh.getNoOfVertices();

	const size_t vSize = _vecVertices.size();
	const size_t iSize = _vecIndices.size();

	_vecVertices.reserve(vSize + nVertices);
	_vecIndices.reserve(iSize + nIndices);

	for (size_t i = 0; i < nVertices; ++i) {
		_vecVertices.push_back(vertices[i]);
	}
	for (size_t i = 0; i < nIndices; ++i) {
		// offset by the already added vertices
		_vecIndices.push_back(indices[i] + vSize);
	}

	return true;
}

void Mesh::removeUnusedVertices() {
	std::vector<bool> isVertexUsed(_vecVertices.size());
	std::fill(isVertexUsed.begin(), isVertexUsed.end(), false);

	for (size_t triCt = 0u; triCt < _vecIndices.size(); ++triCt) {
		IndexType v = _vecIndices[triCt];
		isVertexUsed[v] = true;
	}

	int noOfUsedVertices = 0;
	std::vector<IndexType> newPos(_vecVertices.size());
	for (size_t vertCt = 0u; vertCt < _vecVertices.size(); ++vertCt) {
		if (core_unlikely(!isVertexUsed[vertCt])) {
			continue;
		}
		_vecVertices[noOfUsedVertices] = _vecVertices[vertCt];
		newPos[vertCt] = noOfUsedVertices;
		++noOfUsedVertices;
	}

	_vecVertices.resize(noOfUsedVertices);

	for (size_t triCt = 0u; triCt < _vecIndices.size(); ++triCt) {
		_vecIndices[triCt] = newPos[_vecIndices[triCt]];
	}
	_vecIndices.resize(_vecIndices.size());
}

}
