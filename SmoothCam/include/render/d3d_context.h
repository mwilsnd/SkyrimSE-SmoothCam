#pragma once
#include <d3d11.h>
#include <winrt/base.h>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Render {
	typedef struct _D3DContext {
		// @Note: we don't want to refCount the swap chain - Let skyrim manage the lifetime.
		// As long as the game is running, we have a valid swapchain.
		IDXGISwapChain* swapChain = nullptr;
		winrt::com_ptr<ID3D11Device> device;
		winrt::com_ptr<ID3D11DeviceContext> context;
		// Size of the output window in pixels
		glm::vec2 windowSize = {};
	} D3DContext;

	// The present hook
	typedef HRESULT(*D3D11Present)(IDXGISwapChain*, UINT, UINT);
	HRESULT Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

	// Install D3D hooks
	void InstallHooks();
	// Shutdown, release references
	void Shutdown();
	// Get the game's D3D context
	D3DContext& GetContext() noexcept;
	// Returns true if we have a valid D3D context
	bool HasContext() noexcept;
	// Get the game's depth-stencil view for the back buffer
	winrt::com_ptr<ID3D11DepthStencilView>& GetDepthStencilView() noexcept;
	// Get the game's render target for the back buffer
	winrt::com_ptr<ID3D11RenderTargetView>& GetGameRT() noexcept;

	// Add a new function for drawing during the present hook
	using DrawFunc = std::function<void(D3DContext&)>;
	void OnPresent(DrawFunc&&callback);

	// Set the depth state
	void SetDepthState(D3DContext& ctx, bool writeEnable, bool testEnable, D3D11_COMPARISON_FUNC testFunc);
	// Set the blending state
	void SetBlendState(D3DContext& ctx, bool enable, D3D11_BLEND_OP blendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
		D3D11_BLEND_OP blendAlphaOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
		D3D11_BLEND src = D3D11_BLEND::D3D11_BLEND_ONE, D3D11_BLEND dest = D3D11_BLEND::D3D11_BLEND_ZERO,
		D3D11_BLEND srcAlpha = D3D11_BLEND::D3D11_BLEND_ONE, D3D11_BLEND destAlpha = D3D11_BLEND::D3D11_BLEND_ZERO,
		bool alphaToCoverage = false
	);
	// Set the raster state
	void SetRasterState(D3DContext& ctx, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCCW,
		int32_t depthBias = 0, float depthBiasClamp = 0.0f, float slopeScaledDepthBias = 0.0f, bool lineAA = true,
		bool depthClip = false, bool scissorEnable = false, bool msaa = false
	);

	// Ideally you would do something a bit more clever, but with what little rendering we do storing a lazy cache like this is fine
	// Depth
	typedef struct DSStateKey {
		bool write;
		bool test;
		D3D11_COMPARISON_FUNC mode;

		DSStateKey(bool write, bool test, D3D11_COMPARISON_FUNC mode) :
			write(write), test(test), mode(mode)
		{}

		size_t Hash() const {
			return std::hash<bool>()(write) ^ std::hash<bool>()(test) ^
				std::hash<uint16_t>()(mode);
		}

		bool operator==(const DSStateKey& other) const {
			return write == other.write && test == other.test &&
				mode == other.mode;
		}
	} DSStateKey;

	typedef struct DSState {
		winrt::com_ptr<ID3D11DepthStencilState> state;

		DSState(Render::D3DContext& ctx, DSStateKey& info) {
			D3D11_DEPTH_STENCIL_DESC dsDesc;
			dsDesc.DepthEnable = info.write;
			dsDesc.DepthWriteMask = info.test ? D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL :
				D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
			dsDesc.DepthFunc = info.mode;
			dsDesc.StencilEnable = false;
			ctx.device->CreateDepthStencilState(&dsDesc, state.put());
		}

		~DSState() {}

		DSState(const DSState&) = delete;
		DSState(DSState&& loc) {
			// Doing this, we don't do extra ref counting
			state.attach(loc.state.get());
			loc.state.detach();
		};
		DSState& operator=(const DSState&) = delete;
		DSState& operator=(DSState&& loc) {
			// Doing this, we don't do extra ref counting
			state.attach(loc.state.get());
			loc.state.detach();
		};
	} DSState;

	struct DSHasher {
		size_t operator()(const DSStateKey& key) const {
			return key.Hash();
		}
	};

	struct DSCompare {
		size_t operator()(const DSStateKey& k1, const DSStateKey& k2) const {
			return k1 == k2;
		}
	};

	// Fixed function blending
	typedef struct BlendStateKey {
		D3D11_BLEND_DESC desc;
		float factors[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		BlendStateKey() {
			ZeroMemory(&desc, sizeof (D3D11_BLEND_DESC));
		}

		size_t HashRTBlendDesc(const D3D11_RENDER_TARGET_BLEND_DESC& rtDesc) const {
			return std::hash<bool>()(rtDesc.BlendEnable) ^ std::hash<uint8_t>()(rtDesc.SrcBlend) ^ std::hash<uint8_t>()(rtDesc.DestBlend)
				^ std::hash<uint8_t>()(rtDesc.SrcBlendAlpha) ^ std::hash<uint8_t>()(rtDesc.DestBlendAlpha)
				^ std::hash<uint8_t>()(rtDesc.BlendOp) ^ std::hash<uint8_t>()(rtDesc.BlendOpAlpha)
				^ std::hash<uint8_t>()(rtDesc.RenderTargetWriteMask);
		}

		size_t Hash() const {
			return std::hash<bool>()(desc.AlphaToCoverageEnable) ^ std::hash<bool>()(desc.IndependentBlendEnable) ^
				HashRTBlendDesc(desc.RenderTarget[0]); // @NOTE: We only work with the main RT right now
		}

		bool RTBlendDescEq(const D3D11_RENDER_TARGET_BLEND_DESC& other) const {
			return
				desc.RenderTarget[0].BlendEnable == other.BlendEnable &&
				desc.RenderTarget[0].BlendOp == other.BlendOp &&
				desc.RenderTarget[0].BlendOpAlpha == other.BlendOpAlpha &&
				desc.RenderTarget[0].DestBlend == other.DestBlend &&
				desc.RenderTarget[0].DestBlendAlpha == other.DestBlendAlpha &&
				desc.RenderTarget[0].RenderTargetWriteMask == other.RenderTargetWriteMask &&
				desc.RenderTarget[0].SrcBlend == other.SrcBlend &&
				desc.RenderTarget[0].SrcBlendAlpha == other.SrcBlendAlpha;
		}

		bool operator==(const BlendStateKey& other) const {
			return desc.AlphaToCoverageEnable == other.desc.AlphaToCoverageEnable &&
				desc.IndependentBlendEnable == other.desc.IndependentBlendEnable &&
				RTBlendDescEq(other.desc.RenderTarget[0]);
		}
	} BlendStateKey;

	struct BlendStateHasher {
		size_t operator()(const BlendStateKey& key) const {
			return key.Hash();
		}
	};

	struct BlendStateCompare {
		size_t operator()(const BlendStateKey& k1, const BlendStateKey& k2) const {
			return k1 == k2;
		}
	};

	typedef struct BlendState {
		winrt::com_ptr<ID3D11BlendState> state;

		BlendState(Render::D3DContext& ctx, BlendStateKey& info) {
			ctx.device->CreateBlendState(&info.desc, state.put());
		}

		~BlendState() {}

		BlendState(const BlendState&) = delete;
		BlendState(BlendState&& loc) {
			// Doing this, we don't do extra ref counting
			state.attach(loc.state.get());
			loc.state.detach();
		};
		BlendState& operator=(const BlendState&) = delete;
		BlendState& operator=(BlendState&& loc) {
			// Doing this, we don't do extra ref counting
			state.attach(loc.state.get());
			loc.state.detach();
		}
	} BlendState;
	
	// Rasterizer
	typedef struct RasterStateKey {
		D3D11_RASTERIZER_DESC desc;

		RasterStateKey(D3D11_RASTERIZER_DESC desc) : desc(desc) {}

		size_t Hash() const {
			return std::hash<uint8_t>()(desc.FillMode) ^ std::hash<uint8_t>()(desc.CullMode) ^
				std::hash<bool>()(desc.FrontCounterClockwise) ^ std::hash<uint32_t>()(desc.DepthBias) ^
				std::hash<float>()(desc.DepthBiasClamp) ^ std::hash<float>()(desc.SlopeScaledDepthBias) ^
				std::hash<bool>()(desc.DepthClipEnable) ^ std::hash<bool>()(desc.ScissorEnable) ^
				std::hash<bool>()(desc.MultisampleEnable) ^ std::hash<bool>()(desc.AntialiasedLineEnable);
		}

		bool operator==(const RasterStateKey& other) const {
			return
				desc.FillMode == other.desc.FillMode &&
				desc.CullMode == other.desc.CullMode &&
				desc.FrontCounterClockwise == other.desc.FrontCounterClockwise &&
				desc.DepthBias == other.desc.DepthBias &&
				desc.DepthBiasClamp == other.desc.DepthBiasClamp &&
				desc.DepthClipEnable == other.desc.DepthClipEnable &&
				desc.ScissorEnable == other.desc.ScissorEnable &&
				desc.MultisampleEnable == other.desc.MultisampleEnable &&
				desc.AntialiasedLineEnable == other.desc.AntialiasedLineEnable;
		}
	} RasterStateKey;

	typedef struct RasterState {
		winrt::com_ptr<ID3D11RasterizerState> state;

		RasterState(Render::D3DContext& ctx, RasterStateKey& info) {
			ctx.device->CreateRasterizerState(&info.desc, state.put());
		}

		~RasterState() {}

		RasterState(const RasterState&) = delete;
		RasterState(RasterState&& loc) {
			// Doing this, we don't do extra ref counting
			state.attach(loc.state.get());
			loc.state.detach();
		};
		RasterState& operator=(const RasterState&) = delete;
		RasterState& operator=(RasterState&& loc) {
			// Doing this, we don't do extra ref counting
			state.attach(loc.state.get());
			loc.state.detach();
		};
	} RasterState;

	struct RasterStateHasher {
		size_t operator()(const RasterStateKey& key) const {
			return key.Hash();
		}
	};

	struct RasterStateCompare {
		size_t operator()(const RasterStateKey& k1, const RasterStateKey& k2) const {
			return k1 == k2;
		}
	};
}