#pragma once

namespace Render {
	namespace Model {
#pragma pack(push, 1)
		typedef struct Vertex {
			glm::vec3 position;
			glm::vec2 uv;
			glm::vec3 normal;
			glm::vec4 color;
		} Vertex;

		typedef struct MeshHeader {
			const char name[128];
			uint32_t numVertices;
		} MeshHeader;

		typedef struct ModelHeader {
			uint32_t tag;
			uint32_t version;
			uint32_t numMeshes;
		} ModelHeader;
#pragma pack(pop)

		typedef struct Mesh {
			MeshHeader header;
			std::vector<Vertex> vertices;
		} Mesh;

		typedef struct Model {
			ModelHeader header;
			std::vector<Mesh> meshes;
		} Model;

		bool Load(const uint8_t* location, Model& output);
		void Release(Model& mdl);
	}
}