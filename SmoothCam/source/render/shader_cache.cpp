#include "render/shader_cache.h"

Render::ShaderCache::ShaderCache() noexcept {}

void Render::ShaderCache::Release() noexcept {
	shaders.clear();
}

eastl::shared_ptr<Render::Shader> Render::ShaderCache::Load(const ShaderCreateInfo& info, Render::D3DContext& ctx)
	noexcept
{
	const auto it = shaders.find(info);
	if (it != shaders.end()) {
		if (auto ptr = it->second.lock(); ptr != nullptr) {
			return ptr;
		} else {
			shaders.erase(it);
		}
	}

	auto ptr = eastl::make_shared<Render::Shader>(info, ctx);
	if (ptr->IsValid())
		shaders.insert({ info, ptr });
	return ptr;
}