#include "render/model.h"

bool Render::Model::Load(const uint8_t* location, Model& output) {
	output.header = *reinterpret_cast<const ModelHeader*>(location);
	if (output.header.tag != 0xF00590DA) return false;
	if (output.header.version != 1) return false;

	constexpr auto headerSize = sizeof(ModelHeader);
	constexpr auto meshHeaderSize = sizeof(MeshHeader);
	constexpr auto vertexSize = sizeof(Vertex);

	size_t offset = headerSize;
	for (uint32_t i = 0; i < output.header.numMeshes; i++) {
		const auto base = reinterpret_cast<intptr_t>(location);
		const auto h = reinterpret_cast<const MeshHeader*>(base + offset);
		offset += meshHeaderSize;

		Mesh mesh = {};
		memcpy(&mesh.header, h, meshHeaderSize);
		mesh.vertices.reserve(mesh.header.numVertices);

		for (uint32_t j = 0; j < mesh.header.numVertices; j++) {
			mesh.vertices.push_back(*reinterpret_cast<const Vertex*>(base + offset));
			offset += vertexSize;
		}

		output.meshes.push_back(mesh);
	}

	return true;
};

void Render::Model::Release(Model& mdl) {
	mdl.header.numMeshes = 0;
	mdl.meshes.clear();
	mdl.meshes.shrink_to_fit();
}