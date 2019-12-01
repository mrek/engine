/**
 * @file
 */

#include "AnimationCache.h"

namespace animation {

bool AnimationCache::load(const std::string& filename, size_t meshIndex, const voxel::Mesh* (&meshes)[AnimationSettings::MAX_ENTRIES]) {
	voxel::Mesh& mesh = cacheEntry(filename.c_str());
	if (mesh.getNoOfVertices() > 0) {
		meshes[meshIndex] = &mesh;
		return true;
	}
	if (loadMesh(filename.c_str(), mesh)) {
		meshes[meshIndex] = &mesh;
		return true;
	}
	meshes[meshIndex] = nullptr;
	return false;
}

bool AnimationCache::getMeshes(const AnimationSettings& settings, const voxel::Mesh* (&meshes)[AnimationSettings::MAX_ENTRIES],
		std::function<bool(const voxel::Mesh* (&meshes)[AnimationSettings::MAX_ENTRIES])> loadAdditional) {
	for (size_t i = 0; i < AnimationSettings::MAX_ENTRIES; ++i) {
		if (settings.paths[i].empty()) {
			meshes[i] = nullptr;
			continue;
		}
		const std::string& fullPath = settings.fullPath(i);
		if (!load(fullPath, i, meshes)) {
			Log::error("Failed to load %s", fullPath.c_str());
			return false;
		}
	}
	if (loadAdditional && !loadAdditional(meshes)) {
		return false;
	}
	return true;
}

bool AnimationCache::getModel(const char *fullPath, BoneId bid, Vertices& vertices, Indices& indices) {
	voxel::Mesh& mesh = cacheEntry(fullPath);
	if (mesh.getNoOfVertices() <= 0) {
		if (!loadMesh(fullPath, mesh)) {
			return false;
		}
	}

	vertices.clear();
	indices.clear();

	// TODO: map to array index instead of enum index
	const uint8_t boneIdInt = std::enum_value(bid);
	vertices.reserve(mesh.getNoOfVertices());
	for (const voxel::VoxelVertex& v : mesh.getVertexVector()) {
		vertices.emplace_back(Vertex{v.position, v.colorIndex, boneIdInt, v.ambientOcclusion});
	}
	//vertices.resize(mesh.getNoOfVertices());

	indices.reserve(mesh.getNoOfIndices());
	for (voxel::IndexType idx : mesh.getIndexVector()) {
		indices.push_back((IndexType)idx);
	}
	//indices.resize(mesh.getNoOfIndices());

	return true;
}

bool AnimationCache::getBoneModel(const AnimationSettings& settings, Vertices& vertices, Indices& indices,
		std::function<bool(const voxel::Mesh* (&meshes)[AnimationSettings::MAX_ENTRIES])> loadAdditional) {
	const voxel::Mesh* meshes[AnimationSettings::MAX_ENTRIES] {};
	getMeshes(settings, meshes, loadAdditional);

	vertices.clear();
	indices.clear();
	vertices.reserve(3000);
	indices.reserve(5000);
	IndexType indexOffset = (IndexType)0;
	int meshCount = 0;
	// merge everything into one buffer
	for (size_t i = 0; i < AnimationSettings::MAX_ENTRIES; ++i) {
		const voxel::Mesh *mesh = meshes[i];
		if (mesh == nullptr) {
			continue;
		}
		const BoneIds& bids = settings.boneIds(i);
		core_assert_msg(bids.num >= 0 && bids.num <= 2,
				"number of bone ids is invalid: %i (for mesh type %i)",
				(int)bids.num, (int)i);
		for (uint8_t b = 0u; b < bids.num; ++b) {
			// TODO: map to array index instead of enum index
			const uint8_t boneId = bids.bones[b];
			const std::vector<voxel::VoxelVertex>& meshVertices = mesh->getVertexVector();
			for (const voxel::VoxelVertex& v : meshVertices) {
				vertices.emplace_back(Vertex{v.position, v.colorIndex, boneId, v.ambientOcclusion});
			}

			const std::vector<voxel::IndexType>& meshIndices = mesh->getIndexVector();
			if (bids.mirrored[b]) {
				// if a model is mirrored, this is usually acchieved with negative scaling values
				// thus we have to reverse the winding order here to make the face culling work again
				for (auto i = meshIndices.rbegin(); i != meshIndices.rend(); ++i) {
					indices.push_back((IndexType)(*i) + indexOffset);
				}
			} else {
				for (voxel::IndexType idx : meshIndices) {
					indices.push_back((IndexType)idx + indexOffset);
				}
			}
			indexOffset = (IndexType)vertices.size();
		}
		++meshCount;
	}
	indices.resize(indices.size());
	core_assert(indices.size() % 3 == 0);

	return true;
}

}