#pragma once
#include <d3d11.h>
#include <winrt/base.h>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include "util.h"

namespace Render {
	struct D3DContext {
		// @Note: we don't want to refCount the swap chain - Let skyrim manage the lifetime.
		// As long as the game is running, we have a valid swapchain.
		IDXGISwapChain* swapChain = nullptr;
		winrt::com_ptr<ID3D11Device> device = nullptr;
		winrt::com_ptr<ID3D11DeviceContext> context;
		// Size of the output window in pixels
		glm::vec2 windowSize = {};
		HWND hWnd = nullptr;
	};

	// 143025f00:524728
	struct D3D11Resources {
		int32_t unk0;						// 0x0
		int32_t unk4;						// 0x4
		int32_t unk8;						// 0x8
		int32_t unkC;						// 0xC
		int32_t unk10;						// 0x10
		int32_t unk14;						// 0x14
		int32_t unk18;						// 0x18
		int32_t unk1C;						// 0x1C
		uintptr_t unk20;					// 0x20
		uintptr_t unk28;					// 0x28
		uintptr_t unk30;					// 0x30
		ID3D11Device* device;				// 0x38
		ID3D11DeviceContext* ctx;			// 0x40
		HWND window;						// 0x48
		DWORD windowX;						// 0x50
		DWORD windowY;						// 0x54
		DWORD windowW;						// 0x58
		DWORD windowH;						// 0x5C
		IDXGISwapChain* swapChain;			// 0x60
		uintptr_t unk68;					// 0x68
		uintptr_t unk70;					// 0x70
		ID3D11RenderTargetView* unkRTV78;	// 0x78
		ID3D11ShaderResourceView* unkSRV80;	// 0x80
		uintptr_t unk88;					// 0x88
		uintptr_t unk90;					// 0x90
		uintptr_t unk98;					// 0x98
		uintptr_t unkA0;					// 0xA0
		uintptr_t unkA8;					// 0xA8
		uintptr_t unkB0;					// 0xB0
	};
	static_assert(offsetof(D3D11Resources, unkC) == 0xC);
	static_assert(offsetof(D3D11Resources, unk1C) == 0x1C);
	static_assert(offsetof(D3D11Resources, device) == 0x38);
	static_assert(offsetof(D3D11Resources, swapChain) == 0x60);
	static_assert(offsetof(D3D11Resources, unkB0) == 0xB0);

	// 12: cbuffer12 Buffer 1196 0 - 4096 41 Variables, 656 bytes needed, 720 provided
	struct CBuffer12 {
		// Lots of duplicates
		glm::mat4 unk0;
		glm::mat4 proj0;
		glm::mat4 projView0;
		glm::mat4 projView1;
		glm::mat4 projView2;
		glm::mat4 frustum0;
		glm::mat4 proj1;
		glm::mat4 unk0Transposed;
		glm::mat4 unk2;
		glm::mat4 frustum1;
		glm::mat4 pad8;
		glm::vec4 pad9;

		static ID3D11Buffer* Get() {
			// DAT_143027e88
			return *Offsets::Get<ID3D11Buffer**>(524768);
		}

		static void Set(ID3D11Buffer* buf) {
			*Offsets::Get<ID3D11Buffer**>(524768) = buf;
		}
	};
	static_assert(sizeof(CBuffer12) == 720);
	static_assert(sizeof(CBuffer12) % 16 == 0);

	typedef struct BatchRenderCommand {
		using BSEffectShader = void;
		BSEffectShader* shader;
		BSEffectShaderProperty* properties;
		BSGeometry* geometry;
	} BatchRenderCommand;

	typedef struct GBuffer {
		NiSourceTexture* projectedNoise;
		NiSourceTexture* projectedDiffuse;
		NiSourceTexture* projectedNormal;
		NiSourceTexture* projectedNormalDetail;

		typedef struct WrappedCameraData {
			NiCamera* camera;
			// @Note: This might just be junk on the stack and not actually
			// part of the return type - That said messing with these matrices
			// does end up being transmitted to cbuffer 12
			uintptr_t pad0;
			glm::vec4 dc0;
			glm::vec4 dc1;
			glm::vec4 dc2;
			glm::vec4 dc3;
			glm::vec4 dc4;
			glm::vec4 dc5;
			glm::vec4 dc6;
			glm::mat4 proj;
			glm::mat4 projView0;
			glm::mat4 projView1;
			glm::mat4 projView2;
			glm::mat4 projView3;
		} WrappedCameraData;

		// Called by UpdateGPUCameraData
		WrappedCameraData* CameraSwap(NiCamera* inCamera, byte flags = 0x0);
		// Compute new contents of cbuffer 12 and update on the GPU
		void UpdateGPUCameraData(NiCamera* inCamera, byte flags = 0x0);

		static GBuffer* Get() {
			// DAT_14302c890
			static auto gbuffer = Offsets::Get<Render::GBuffer*>(524998);
			return gbuffer;
		}

		// Other locations of interest:
		// FUN_1412e3520:100421
		// typedef void(*DrawEarlyZPass)(bool, bool);

		// FUN_1412e3e70:100424
		// typedef void(*DrawGBuffer)(char param_1);

		// BSBatchRenderer::RenderBatch::FUN_141308440:100854
		// typedef void(*RenderStuff)(Render::BatchRenderCommand* cmd, uint32_t id, bool unk0, uint32_t unk1);

		// SkyrimSE.exe+0x00d6cbc7 <- Draw
		// SkyrimSE.exe+0x00c7cb14 <- FUN_140c7ca70_RenderSomething::vtable.NiSkinPartition
		// SkyrimSE.exe+0x00c6ba5f <- FUN_140c6b9f0::vtable.BSDismemberSkinInstance::Draw (One of many)
		// SkyrimSE.exe+0x01308a97 <- CALL qword ptr [RAX + 0x128] CommonLibSSE : BSDismemberSkinInstance->Unk_25
		// SkyrimSE.exe+<many>     <- ?
		// SkyrimSE.exe+0x013082d3 <- BSBatchRenderer::RenderBatch::FUN_141308440
		// SkyrimSE.exe+0x012ccf4f <- BSShaderAccumulator::FUN_141308030
		// SkyrimSE.exe+<many>     <- ?
		// SkyrimSE.exe+0x012c1672 <- NiCamera::FUN_140d7bab0:UpdateCameraDataAndCBuffer12
	} GBuffer;

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
	void OnPresent(DrawFunc&& callback);

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
			size_t seed = 0;
			Util::HashCombine(seed, write);
			Util::HashCombine(seed, test);
			Util::HashCombine(seed, mode);
			return seed;
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
			size_t seed = 0;
			Util::HashCombine(seed, rtDesc.BlendEnable);
			Util::HashCombine(seed, rtDesc.SrcBlend);
			Util::HashCombine(seed, rtDesc.DestBlend);
			Util::HashCombine(seed, rtDesc.SrcBlendAlpha);
			Util::HashCombine(seed, rtDesc.DestBlendAlpha);
			Util::HashCombine(seed, rtDesc.BlendOp);
			Util::HashCombine(seed, rtDesc.BlendOpAlpha);
			Util::HashCombine(seed, rtDesc.RenderTargetWriteMask);
			return seed;
		}

		size_t Hash() const {
			size_t seed = 0;
			Util::HashCombine(seed, desc.AlphaToCoverageEnable);
			Util::HashCombine(seed, desc.IndependentBlendEnable);
			Util::HashCombine(seed, HashRTBlendDesc(desc.RenderTarget[0])); // @NOTE: We only work with the main RT right now
			return seed;
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
			size_t seed = 0;
			Util::HashCombine(seed, desc.FillMode);
			Util::HashCombine(seed, desc.CullMode);
			Util::HashCombine(seed, desc.FrontCounterClockwise);
			Util::HashCombine(seed, desc.DepthBias);
			Util::HashCombine(seed, desc.DepthBiasClamp);
			Util::HashCombine(seed, desc.SlopeScaledDepthBias);
			Util::HashCombine(seed, desc.DepthClipEnable);
			Util::HashCombine(seed, desc.ScissorEnable);
			Util::HashCombine(seed, desc.MultisampleEnable);
			Util::HashCombine(seed, desc.AntialiasedLineEnable);
			return seed;
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