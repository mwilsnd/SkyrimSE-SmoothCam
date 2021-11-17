#ifdef WITH_D2D
#include "render/d2d.h"
#include "render/render_target.h"
#include "render/texture2d.h"
#include "render/srv.h"
#include "render/vertex_buffer.h"
#include "render/shaders/draw_fullscreen_texture.h"
#include "render/shader_cache.h"

Render::D2D::D2D(D3DContext& ctx) {
	D2D1_FACTORY_OPTIONS options;
	options.debugLevel = D2D1_DEBUG_LEVEL_NONE;

	if (!SUCCEEDED(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		__uuidof(ID2D1Factory2),
		factory.put_void()
	)))
	{
		FatalError(L"SmoothCam: Failed to initialize direct2d");
	}

	DXGI_SWAP_CHAIN_DESC desc;
	ctx.swapChain->GetDesc(&desc);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0
	};
	uint32_t numLevels = 1;

	// We need to be on the same adapter as the game device for texture sharing
	winrt::com_ptr<IDXGIDevice> gameDev;
	if (!SUCCEEDED(ctx.device->QueryInterface(__uuidof(IDXGIDevice), gameDev.put_void()))) {
		FatalError(L"SmoothCam: Failed to get DXGI device");
	}

	winrt::com_ptr<IDXGIAdapter> gameAdapter;
	if (!SUCCEEDED(gameDev->GetAdapter(gameAdapter.put()))) {
		FatalError(L"SmoothCam: Failed to initialize direct2d");
	}

	// And the whole reason we have to make our own device
	uint32_t flags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	if (!SUCCEEDED(D3D11CreateDevice(
		gameAdapter.get(),
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		flags,
		featureLevels,
		numLevels,
		D3D11_SDK_VERSION,
		d2dContext.device.put(),
		nullptr,
		d2dContext.context.put()
	))) {
		FatalError(L"SmoothCam: Failed to create d3d device for direct2d");
	}

	// Now get the dxgi device
	if (!SUCCEEDED(d2dContext.device->QueryInterface(__uuidof(IDXGIDevice), dxgiDevice.put_void()))) {
		FatalError(L"SmoothCam: Failed to get DXGI device");
	}

	if (!SUCCEEDED(factory->CreateDevice(dxgiDevice.get(), device.put()))) {
		FatalError(L"SmoothCam: Failed to create direct2d device");
	}

	if (!SUCCEEDED(device->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
		context.put()
	)))
	{
		FatalError(L"SmoothCam: Failed to create direct2d device context");
	}

	// Make our target, setup texture sharing
	D2D1_BITMAP_PROPERTIES1 props;
	ZeroMemory(&props, sizeof(D2D1_BITMAP_PROPERTIES1));
	props.dpiX = 96;
	props.dpiY = 96;
	props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	props.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

	CreateRenderTarget(d2dContext, ctx);

	if (!SUCCEEDED(colorBuffer->GetResource()->QueryInterface(__uuidof(IDXGISurface), dxgiBackBuffer.put_void()))) {
		FatalError(L"SmoothCam: Failed to get buffer surface");
	}

	auto code = context->CreateBitmapFromDxgiSurface(dxgiBackBuffer.get(), props, bitmap.put());
	if (!SUCCEEDED(code)) {
		FatalError(L"SmoothCam: Failed to create direct2d bitmap");
	}

	context->SetTarget(bitmap.get());

	// Start direct write
	dwrite = eastl::make_unique<Render::DWrite>(this);

	// Make a work query for forcing sync on the shared texture
	D3D11_QUERY_DESC qd;
	qd.MiscFlags = 0;
	qd.Query = D3D11_QUERY::D3D11_QUERY_EVENT;
	if (!SUCCEEDED(d2dContext.device->CreateQuery(&qd, workQuery.put()))) {
		FatalError(L"SmoothCam: Failed to create device sync object");
	}

	// Fullscreen vertex buffer for drawing d2d on the game back buffer
	Render::ShaderCreateInfo vsCreateInfo(
		Render::Shaders::DrawFullscreenTextureVS,
		Render::PipelineStage::Vertex
	);
	fullScreenVS = ShaderCache::Get().Load(vsCreateInfo, ctx);

	Render::ShaderCreateInfo psCreateInfo(
		Render::Shaders::DrawFullscreenTexturePS,
		Render::PipelineStage::Fragment
	);
	fullScreenPS = ShaderCache::Get().Load(psCreateInfo, ctx);

	float verts[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,

		-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
	};

	Render::VertexBufferCreateInfo vbInfo;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &verts;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	vbInfo.elementSize = sizeof(float) * 5;
	vbInfo.numElements = 6;
	vbInfo.elementData = &data;
	vbInfo.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	vbInfo.bufferUsage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	vbInfo.cpuAccessFlags = 0;
	vbInfo.vertexProgram = fullScreenVS;
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{
		"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		0, D3D11_INPUT_PER_VERTEX_DATA, 0
	});
	vbInfo.iaLayout.emplace_back(D3D11_INPUT_ELEMENT_DESC{
		"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
	});
	vboFullscreen = eastl::make_unique<Render::VertexBuffer>(vbInfo, ctx);

	// Init our different stroke styles
	strokeStyles[static_cast<size_t>(StrokeStyle::Solid)] = GetStrokeStyle<0>(
		nullptr,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_LINE_JOIN::D2D1_LINE_JOIN_MITER,
		D2D1_DASH_STYLE::D2D1_DASH_STYLE_SOLID
	);
	strokeStyles[static_cast<size_t>(StrokeStyle::RoundedSolid)] = GetStrokeStyle<0>(
		nullptr,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_ROUND,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_LINE_JOIN::D2D1_LINE_JOIN_MITER,
		D2D1_DASH_STYLE::D2D1_DASH_STYLE_SOLID
	);
	strokeStyles[static_cast<size_t>(StrokeStyle::RoundedSolidRounded)] = GetStrokeStyle<0>(
		nullptr,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_ROUND,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_LINE_JOIN::D2D1_LINE_JOIN_ROUND,
		D2D1_DASH_STYLE::D2D1_DASH_STYLE_SOLID
	);

	const float dashes[2] = { 5.0f, 5.0f };
	strokeStyles[static_cast<size_t>(StrokeStyle::Dashed)] = GetStrokeStyle<2>(
		dashes,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_LINE_JOIN::D2D1_LINE_JOIN_MITER,
		D2D1_DASH_STYLE::D2D1_DASH_STYLE_CUSTOM
	);
	strokeStyles[static_cast<size_t>(StrokeStyle::RoundedDashed)] = GetStrokeStyle<2>(
		dashes,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_ROUND,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_LINE_JOIN::D2D1_LINE_JOIN_MITER,
		D2D1_DASH_STYLE::D2D1_DASH_STYLE_CUSTOM
	);
	strokeStyles[static_cast<size_t>(StrokeStyle::RoundedDashedRounded)] = GetStrokeStyle<2>(
		dashes,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_ROUND,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_FLAT,
		D2D1_LINE_JOIN::D2D1_LINE_JOIN_ROUND,
		D2D1_DASH_STYLE::D2D1_DASH_STYLE_CUSTOM
	);
	strokeStyles[static_cast<size_t>(StrokeStyle::RoundedDashedRoundedSmooth)] = GetStrokeStyle<2>(
		dashes,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_ROUND,
		D2D1_CAP_STYLE::D2D1_CAP_STYLE_ROUND,
		D2D1_LINE_JOIN::D2D1_LINE_JOIN_ROUND,
		D2D1_DASH_STYLE::D2D1_DASH_STYLE_CUSTOM
	);
}

Render::D2D::~D2D() {
	colorBrushes.clear();

	for (auto& ptr : strokeStyles)
		ptr = nullptr;

	dwrite.reset();
	bitmap = nullptr;

	vboFullscreen.reset();
	fullScreenVS.reset();
	fullScreenPS.reset();

	colorSRV.reset();
	dxgiBackBuffer = nullptr;
	sharedColorBuffer.reset();
	colorBuffer.reset();

	context = nullptr;
	device = nullptr;
	dxgiDevice = nullptr;
	factory = nullptr;

	d2dContext.context = nullptr;
	d2dContext.device = nullptr;
}

void Render::D2D::BeginFrame() noexcept {
	context->BeginDraw();
	context->Clear();
}

void Render::D2D::EndFrame() noexcept {
	D2D1_TAG t1, t2;
	context->Flush(&t1, &t2);
	context->EndDraw();
	d2dContext.context->End(workQuery.get());
}

void Render::D2D::FlushAndSpin() noexcept {
	d2dContext.context->Flush();

	BOOL deviceDone;
	do {
		const auto code = d2dContext.context->GetData(workQuery.get(), &deviceDone, sizeof(BOOL), 0);
		if (code == S_OK) break;
	} while (true);
}

void Render::D2D::WriteToBackbuffer(D3DContext& ctx) noexcept {
	sharedColorBuffer->Bind(ctx, Render::PipelineStage::Fragment, 0);
	colorSRV->Bind(ctx, Render::PipelineStage::Fragment, 0);
	fullScreenVS->Use();
	fullScreenPS->Use();
	vboFullscreen->Bind(0);

	Render::SetDepthState(ctx, false, false, D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS);
	Render::SetBlendState(
		ctx, true,
		D3D11_BLEND_OP::D3D11_BLEND_OP_ADD, D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
		D3D11_BLEND::D3D11_BLEND_SRC_ALPHA, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND::D3D11_BLEND_ONE, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA
	);
	Render::SetRasterState(ctx, D3D11_FILL_MODE::D3D11_FILL_SOLID, D3D11_CULL_MODE::D3D11_CULL_NONE, true);

	vboFullscreen->Draw();
}

eastl::unique_ptr<Render::DWrite>& Render::D2D::GetDWrite() noexcept {
	return dwrite;
}

winrt::com_ptr<ID2D1SolidColorBrush> Render::D2D::GetColorBrush(const glm::vec4& color) noexcept {
	auto it = colorBrushes.find(ColorBrushKey{ color });
	if (it != colorBrushes.end()) return it->second;

	winrt::com_ptr<ID2D1SolidColorBrush> brush;
	context->CreateSolidColorBrush(
		{ color.r, color.g, color.b, color.a },
		brush.put()
	);

	colorBrushes.insert({ ColorBrushKey{ color }, brush });
	return brush;
}

void Render::D2D::DrawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color, float thickness) noexcept {
	context->DrawLine({ p1.x, p1.y }, { p2.x, p2.y }, GetColorBrush(color).get(), thickness);
}

void Render::D2D::DrawEllipse(const glm::vec2& center, const glm::vec2& extents, const glm::vec4& color,
	StrokeStyle style, const float stroke) noexcept
{
	D2D1_ELLIPSE el;
	el.point = { center.x, center.y };
	el.radiusX = extents.x;
	el.radiusY = extents.y;
	context->DrawEllipse(el, GetColorBrush(color).get(), stroke, strokeStyles[static_cast<size_t>(style)].get());
}

void Render::D2D::CreateRenderTarget(D3DContext& ctx, D3DContext& renderingCtx) {
	Texture2DCreateInfo texInfo;
	texInfo.width = static_cast<uint32_t>(renderingCtx.windowSize.x);
	texInfo.height = static_cast<uint32_t>(renderingCtx.windowSize.y);
	texInfo.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texInfo.usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	texInfo.bindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE |
		D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	texInfo.miscFlags = D3D11_RESOURCE_MISC_SHARED;
	colorBuffer = eastl::make_shared<Texture2D>(ctx, texInfo);

	winrt::com_ptr<IDXGIResource> res;
	auto code = colorBuffer->GetResource()->QueryInterface(__uuidof(IDXGIResource), res.put_void());
	if (!SUCCEEDED(code)) {
		FatalError(L"SmoothCam: Failed to share direct2d color target with game context");
	}

	HANDLE handle;
	code = res->GetSharedHandle(&handle);
	if (!SUCCEEDED(code) || !handle) {
		FatalError(L"SmoothCam: Failed to share direct2d color target with game context");
	}

	winrt::com_ptr<IDXGIResource> sharedRes;
	winrt::com_ptr<ID3D11Texture2D> sharedTex;
	code = renderingCtx.device->OpenSharedResource(
		handle,
		__uuidof(ID3D11Texture2D),
		sharedRes.put_void()
	);
	if (!SUCCEEDED(code)) {
		FatalError(L"SmoothCam: Failed to share direct2d color target with game context");
	}

	code = sharedRes->QueryInterface(__uuidof(ID3D11Texture2D), sharedTex.put_void());
	if (!SUCCEEDED(code)) {
		FatalError(L"SmoothCam: Failed to share direct2d color target with game context");
	}

	texInfo.createSampler = true;
	sharedColorBuffer = eastl::make_shared<Texture2D>(renderingCtx, sharedTex, texInfo);

	SRVCreateInfo srv;
	srv.dimensions = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
	srv.format = texInfo.format;
	srv.texture = sharedColorBuffer;
	srv.texture2D.MipLevels = 1;
	srv.texture2D.MostDetailedMip = 0;
	colorSRV = eastl::make_unique<Render::SRV>(renderingCtx, srv);
}
#endif