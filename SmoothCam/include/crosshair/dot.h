#pragma once
#include "crosshair/base.h"
#include "render/model.h"
#include "render/mesh_drawer.h"

namespace Crosshair {
	class Dot : public Base {
		public:
			explicit Dot() = default;
			virtual ~Dot() noexcept {};
			Dot(const Dot&) = delete;
			Dot(Dot&&) noexcept = delete;
			Dot& operator=(const Dot&) = delete;
			Dot& operator=(Dot&&) noexcept = delete;

			virtual void Create3D(Render::D3DContext& ctx, eastl::shared_ptr<Render::CBuffer>& perObjectBuf) noexcept override;
			virtual void Render(Render::D3DContext& ctx, bool allowDepthTesting) noexcept override;

		private:
			eastl::unique_ptr<Render::MeshDrawer> meshDrawer;
	};
}