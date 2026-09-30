#pragma once

namespace Render {
	namespace Model {
#pragma pack(push, 1)
		typedef struct Vertex {
			vec3u position = {};
			vec2u uv = {};
			vec3u normal = {};
			vec4u color = {};
		} Vertex;

		typedef struct MeshHeader {
			const char name[128];
			uint32_t numVertices = 0;
		} MeshHeader;

		typedef struct ModelHeader {
			uint32_t tag = 0;
			uint32_t version = 0;
			uint32_t numMeshes = 0;
		} ModelHeader;
#pragma pack(pop)

		typedef struct Mesh {
			MeshHeader header = {};
			std::vector<Vertex> vertices = {};
		} Mesh;

		typedef struct Model {
			ModelHeader header = {};
			std::vector<Mesh> meshes = {};
		} Model;

		bool Load(const uint8_t* location, Model& output);
		void Release(Model& mdl);
	}
}