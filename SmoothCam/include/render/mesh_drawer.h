#pragma once
#include "render/vertex_buffer.h"
#include "render/shader.h"
#include "render/cbuffer.h"
#include "render/model.h"

namespace Render {
	typedef struct MeshCreateInfo {
		Model::Mesh* mesh = nullptr;
		std::shared_ptr<Shader> vs;
		std::shared_ptr<Shader> ps;
	} MeshCreateInfo;

	class MeshDrawer {
		public:
			explicit MeshDrawer(MeshCreateInfo& info, std::shared_ptr<Render::CBuffer>& perObjectBuffer, D3DContext& ctx);
			~MeshDrawer();
			MeshDrawer(const MeshDrawer&) = delete;
			MeshDrawer(MeshDrawer&&) noexcept = delete;
			MeshDrawer& operator=(const MeshDrawer&) = delete;
			MeshDrawer& operator=(MeshDrawer&&) noexcept = delete;

			// Draw the mesh using the given model matrix
			void Submit(glm::mat4& modelMatrix) noexcept;

			// Set the shaders used by the mesh for rendering
			void SetShaders(std::shared_ptr<Shader>& vs, std::shared_ptr<Shader>& ps);

		private:
			D3DContext context;
			std::unique_ptr<VertexBuffer> vbo;
			std::shared_ptr<CBuffer> cbufPerObject;
			std::shared_ptr<Shader> vs;
			std::shared_ptr<Shader> ps;

			void CreateObjects(std::vector<Model::Vertex>& vertices, D3DContext& ctx);
	};
}