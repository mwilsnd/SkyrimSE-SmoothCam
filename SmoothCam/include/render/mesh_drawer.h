#pragma once
#include "render/vertex_buffer.h"
#include "render/shader.h"
#include "render/cbuffer.h"
#include "render/model.h"

namespace Render {
	typedef struct MeshCreateInfo {
		Model::Mesh* mesh = nullptr;
		eastl::shared_ptr<Shader> vs;
		eastl::shared_ptr<Shader> ps;
	} MeshCreateInfo;

	class MeshDrawer {
		public:
			MeshDrawer(MeshCreateInfo& info, const eastl::shared_ptr<Render::CBuffer>& perObjectBuffer, D3DContext& ctx)
				noexcept;
			~MeshDrawer();
			MeshDrawer(const MeshDrawer&) = delete;
			MeshDrawer(MeshDrawer&&) noexcept = delete;
			MeshDrawer& operator=(const MeshDrawer&) = delete;
			MeshDrawer& operator=(MeshDrawer&&) noexcept = delete;

			// Draw the mesh using the given model matrix
			void Submit(const glm::mat4& modelMatrix) noexcept;

			// Set the shaders used by the mesh for rendering
			void SetShaders(eastl::shared_ptr<Shader>& vs, eastl::shared_ptr<Shader>& ps);

		private:
			D3DContext context;
			eastl::unique_ptr<VertexBuffer> vbo;
			eastl::shared_ptr<CBuffer> cbufPerObject;
			eastl::shared_ptr<Shader> vs;
			eastl::shared_ptr<Shader> ps;

			void CreateObjects(eastl::vector<Model::Vertex>& vertices, D3DContext& ctx);
	};
}