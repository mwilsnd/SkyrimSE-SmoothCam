#include "render/d3d_context.h"
#include "render/d2d.h"
#include "render/dwrite.h"
#include "render/render_target.h"
#include "render/shader.h"
#include "render/srv.h"
#include "render/texture2d.h"
#include "render/vertex_buffer.h"
#include "util.h"
#include "debug/eh.h"

static Render::D3DContext gameContext;
static eastl::vector<Render::DrawFunc> presentCallbacks;
static eastl::unique_ptr<VTableDetour<IDXGISwapChain>> dxgiHook;
static bool initialized = false;

#ifdef WITH_D2D
extern eastl::unique_ptr<Render::D2D> g_D2D;
#endif

struct D3DObjectsStore {
	winrt::com_ptr<ID3D11DepthStencilView> depthStencilView;
	winrt::com_ptr<ID3D11RenderTargetView> gameRTV;

	eastl::unordered_map<
		Render::DSStateKey, Render::DSState,
		Render::DSHasher, Render::DSCompare
	> loadedDepthStates;

	eastl::unordered_map<
		Render::BlendStateKey, Render::BlendState,
		Render::BlendStateHasher, Render::BlendStateCompare
	> loadedBlendStates;

	eastl::unordered_map<
		Render::RasterStateKey, Render::RasterState,
		Render::RasterStateHasher, Render::RasterStateCompare
	> loadedRasterStates;

	void release() {
		depthStencilView = nullptr;
		gameRTV = nullptr;
		loadedRasterStates.clear();
		loadedDepthStates.clear();
		loadedBlendStates.clear();
	}
};
static D3DObjectsStore d3dObjects;


HRESULT Render::Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
	const auto mdmp = Debug::MiniDumpScope();

	// Save some context state to restore later
	winrt::com_ptr<ID3D11DepthStencilState> gameDSState;
	uint32_t gameStencilRef;
	gameContext.context->OMGetDepthStencilState(gameDSState.put(), &gameStencilRef);

	winrt::com_ptr<ID3D11BlendState> gameBlendState;
	float gameBlendFactors[4];
	uint32_t gameSampleMask;
	gameContext.context->OMGetBlendState(gameBlendState.put(), gameBlendFactors, &gameSampleMask);

	D3D11_VIEWPORT gamePort;
	uint32_t numPorts = 1;
	gameContext.context->RSGetViewports(&numPorts, &gamePort);

	D3D11_VIEWPORT port;
	port.TopLeftX = gamePort.TopLeftX;
	port.TopLeftY = gamePort.TopLeftY;
	port.Width = gamePort.Width;
	port.Height = gamePort.Height;
	port.MinDepth = 0;
	port.MaxDepth = 1;
	gameContext.context->RSSetViewports(1, &port);

	winrt::com_ptr<ID3D11RasterizerState> rasterState;
	gameContext.context->RSGetState(rasterState.put());

	gameContext.context->OMGetRenderTargets(1, d3dObjects.gameRTV.put(), d3dObjects.depthStencilView.put());

	{
#ifdef WITH_D2D
		g_D2D->BeginFrame();
#endif

		for (auto& cb : presentCallbacks)
			cb(gameContext);

#ifdef WITH_D2D
		g_D2D->EndFrame();
#endif

		auto color = d3dObjects.gameRTV.get();
		gameContext.context->OMSetRenderTargets(1, &color, d3dObjects.depthStencilView.get());

#ifdef WITH_D2D
		g_D2D->FlushAndSpin();
		g_D2D->WriteToBackbuffer(gameContext);
#endif
	}

	// Put things back the way we found it
	gameContext.context->RSSetState(rasterState.get());
	gameContext.context->RSSetViewports(1, &gamePort);
	gameContext.context->OMSetBlendState(gameBlendState.get(), gameBlendFactors, gameSampleMask);
	gameContext.context->OMSetDepthStencilState(gameDSState.get(), gameStencilRef);

	d3dObjects.depthStencilView = nullptr;
	d3dObjects.gameRTV = nullptr;

	return dxgiHook->GetBase<Render::D3D11Present>(8)(swapChain, syncInterval, flags);
}

static bool ReadSwapChain() {
	// Hopefully SEH will shield us from unexpected crashes with other mods/programs
	// @Note: no MiniDumpScope here, we allow for this to fail and simply disable D3D features
	__try {
		auto data = *Offsets::Get<Render::D3D11Resources**>(524728);
		// Naked pointer to the swap chain
		gameContext.swapChain = data->swapChain;
		// Device
		gameContext.device.copy_from(data->device);
		// Context
		gameContext.context.copy_from(data->ctx);

		// Try and read the desc as a simple test
		DXGI_SWAP_CHAIN_DESC desc;
		if (!SUCCEEDED(data->swapChain->GetDesc(&desc)))
			return false;

		gameContext.windowSize.x = static_cast<float>(data->windowW);
		gameContext.windowSize.y = static_cast<float>(data->windowH);
		gameContext.hWnd = data->window;
	} __except (1) {
		return false;
	}
	return true;
}

void Render::InstallHooks() {
	if (!ReadSwapChain()) {
		_ERROR("SmoothCam: Failed to hook IDXGISwapChain::Present and aquire device context, drawing is disabled.");
		return;
	}

	const auto mdmp = Debug::MiniDumpScope();

	dxgiHook = eastl::make_unique<VTableDetour<IDXGISwapChain>>(gameContext.swapChain);
	dxgiHook->Add(8, Render::Present);
	if (!dxgiHook->Attach()) {
		_ERROR("SmoothCam: Failed to place detour on virtual IDXGISwapChain->Present.");
		return;
	}

	initialized = true;
}

void Render::Shutdown() {
	if (!initialized) return;
	initialized = false;

	// Release program lifetime objects
	d3dObjects.release();

	// Free our present hook
	dxgiHook->Detach();

	// Explicit release
	gameContext.context = nullptr;
	gameContext.device = nullptr;
}

Render::D3DContext& Render::GetContext() noexcept {
	return gameContext;
}

bool Render::HasContext() noexcept {
	return initialized;
}

winrt::com_ptr<ID3D11DepthStencilView>& Render::GetDepthStencilView() noexcept {
	return d3dObjects.depthStencilView;
}

winrt::com_ptr<ID3D11RenderTargetView>& Render::GetGameRT() noexcept {
	return d3dObjects.gameRTV;
}

void Render::OnPresent(DrawFunc&& callback) {
	presentCallbacks.emplace_back(callback);
}

void Render::SetDepthState(D3DContext& ctx, bool writeEnable, bool testEnable, D3D11_COMPARISON_FUNC testFunc) {
	auto key = DSStateKey{ writeEnable, testEnable, testFunc };
	auto it = d3dObjects.loadedDepthStates.find(key);
	if (it != d3dObjects.loadedDepthStates.end()) {
		ctx.context->OMSetDepthStencilState(it->second.state.get(), 255);
		return;
	}

	auto state = DSState{ ctx, key };
	ctx.context->OMSetDepthStencilState(state.state.get(), 255);
	d3dObjects.loadedDepthStates.emplace(key, std::move(state));
}

void Render::SetBlendState(D3DContext& ctx, bool enable, D3D11_BLEND_OP blendOp, D3D11_BLEND_OP blendAlphaOp,
	D3D11_BLEND src, D3D11_BLEND dest, D3D11_BLEND srcAlpha, D3D11_BLEND destAlpha, bool alphaToCoverage)
{
	auto key = BlendStateKey{};
	key.desc.AlphaToCoverageEnable = alphaToCoverage;
	key.desc.IndependentBlendEnable = false;
	key.desc.RenderTarget[0].BlendEnable = enable;
	key.desc.RenderTarget[0].BlendOp = blendOp;
	key.desc.RenderTarget[0].BlendOpAlpha = blendAlphaOp;
	key.desc.RenderTarget[0].SrcBlend = src;
	key.desc.RenderTarget[0].DestBlend = dest;
	key.desc.RenderTarget[0].SrcBlendAlpha = srcAlpha;
	key.desc.RenderTarget[0].DestBlendAlpha = destAlpha;
	key.desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

	auto it = d3dObjects.loadedBlendStates.find(key);
	if (it != d3dObjects.loadedBlendStates.end()) {
		ctx.context->OMSetBlendState(it->second.state.get(), key.factors, 0xffffffff);
		return;
	}

	auto state = BlendState{ ctx, key };
	ctx.context->OMSetBlendState(state.state.get(), key.factors, 0xffffffff);

	d3dObjects.loadedBlendStates.emplace(key, std::move(state));
}

void Render::SetRasterState(D3DContext& ctx, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCCW,
	int32_t depthBias, float depthBiasClamp, float slopeScaledDepthBias, bool lineAA, bool depthClip,
	bool scissorEnable, bool msaa)
{
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = fillMode;
	desc.CullMode = cullMode;
	desc.FrontCounterClockwise = frontCCW;
	desc.DepthBias = depthBias;
	desc.DepthBiasClamp = depthBiasClamp;
	desc.SlopeScaledDepthBias = slopeScaledDepthBias;
	desc.DepthClipEnable = depthClip;
	desc.ScissorEnable = scissorEnable;
	desc.MultisampleEnable = msaa;
	desc.AntialiasedLineEnable = lineAA;
	auto key = RasterStateKey{ desc };

	auto it = d3dObjects.loadedRasterStates.find(key);
	if (it != d3dObjects.loadedRasterStates.end()) {
		ctx.context->RSSetState(it->second.state.get());
		return;
	}

	auto state = RasterState{ ctx, key };
	ctx.context->RSSetState(state.state.get());
	d3dObjects.loadedRasterStates.emplace(key, std::move(state));
}

Render::GBuffer::WrappedCameraData* Render::GBuffer::CameraSwap(NiCamera* inCamera, byte flags) {
	// FUN_140d7d7b0
	typedef WrappedCameraData*(*ty)(GBuffer* param_1, NiCamera* param_2, byte param_3);
	static auto fn = Offsets::Get<ty>(75713);
	return fn(this, inCamera, flags);
}

void Render::GBuffer::UpdateGPUCameraData(NiCamera* inCamera, byte flags) {
	// FUN_140d7bab0
	typedef void(*ty)(GBuffer* param_1, NiCamera* param_2, byte param_3);
	static auto fn = Offsets::Get<ty>(75694);
	fn(this, inCamera, flags);
}