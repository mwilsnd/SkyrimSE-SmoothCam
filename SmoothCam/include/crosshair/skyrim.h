#pragma once
#include "crosshair/base.h"
#include "render/model.h"
#include "render/vertex_buffer.h"
#include "render/shader.h"
#include "render/mesh_drawer.h"

namespace Crosshair {
	class Skyrim : public Base {
		public:
			Skyrim() = default;
			virtual ~Skyrim() noexcept {};
			Skyrim(const Skyrim&) = delete;
			Skyrim(Skyrim&&) noexcept = delete;
			Skyrim& operator=(const Skyrim&) = delete;
			Skyrim& operator=(Skyrim&&) noexcept = delete;

			virtual void Create3D(Render::D3DContext& ctx, eastl::shared_ptr<Render::CBuffer>& perObjectBuf) noexcept override;
			virtual void Render(Render::D3DContext& ctx, float curTime, float deltaTime, bool allowDepthTesting) noexcept override;

		private:
			eastl::unique_ptr<Render::MeshDrawer> meshDrawer;
	};
}