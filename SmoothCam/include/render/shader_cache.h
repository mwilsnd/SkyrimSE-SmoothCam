#pragma once
#include "render/shader.h"

namespace Render {
	class ShaderCache {
		public:
			ShaderCache(const ShaderCache&) = delete;
			ShaderCache(ShaderCache&&) noexcept = delete;
			ShaderCache& operator=(const ShaderCache&) = delete;
			ShaderCache& operator=(ShaderCache&&) noexcept = delete;

			static ShaderCache& Get() noexcept {
				static ShaderCache cache;
				return cache;
			}

			void Release() noexcept;

			eastl::shared_ptr<Shader> Load(const ShaderCreateInfo& info, Render::D3DContext& ctx) noexcept;

			struct SCIHasher {
				size_t operator()(const ShaderCreateInfo& key) const {
					return key.Hash();
				}
			};

			struct SCICompare {
				size_t operator()(const ShaderCreateInfo& k1, const ShaderCreateInfo& k2) const {
					return k1 == k2;
				}
			};

		private:
			ShaderCache() noexcept;

		private:
			eastl::unordered_map<
				ShaderCreateInfo, eastl::weak_ptr<Render::Shader>,
				SCIHasher, SCICompare
			> shaders;
	};
}