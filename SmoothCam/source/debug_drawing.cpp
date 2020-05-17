#include "debug_drawing.h"
#include <d3dcompiler.h>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d3dcompiler.lib")

bool initialized = false;
bool drawingEnabled = true;
bool drawTogglePressed = false;
typedef struct {
	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	DWORD_PTR* swapChainVTable = nullptr;
} TempD3DResources;
TempD3DResources tempD3DResources;

DebugDrawing::D3DObjects obj;
DebugDrawing::CommandQueue commandQueue;
struct {
	std::unique_ptr<DebugDrawing::LineDrawer> line;
	std::unique_ptr<DebugDrawing::BoxDrawer> box;
} drawers;

constexpr const auto drawLineVS = R"(
struct VS_INPUT {
	float4 vPos : POS;
	float4 vColor : COL;
};

struct VS_OUTPUT {
	float4 vPos : SV_POSITION;
	float4 vColor : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.vPos = input.vPos;
	output.vColor = input.vColor;
	return output;
}
)";

constexpr const auto drawLinePS = R"(
struct PS_INPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return input.color;
}
)";

#pragma region D3DHook
static PLH::VFuncMap origVFuncs_D3D;
DebugDrawing::D3D11Present fnPresentOrig;
HRESULT DebugDrawing::Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
	if (!initialized) {
		obj.swapChain = swapChain;
		auto ret = swapChain->GetDevice(__uuidof(ID3D11Device), (LPVOID*)&obj.device);
		if (SUCCEEDED(ret))
			obj.device->GetImmediateContext(&obj.context);
		else
			assert(0);

		DebugDrawing::CreateDrawers();
		initialized = true;
	}

	DebugDrawing::Flush();

	return reinterpret_cast<DebugDrawing::D3D11Present>(origVFuncs_D3D[8])(swapChain, syncInterval, flags);
}

bool DebugDrawing::CreateTempResources() {
	auto hWnd = GetForegroundWindow();
	if (hWnd == nullptr)
		return false;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = (GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0 ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
		&tempD3DResources.swapChain, &tempD3DResources.device, NULL, &tempD3DResources.context)))
	{
		return false;
	}

	tempD3DResources.swapChainVTable = (DWORD_PTR*)tempD3DResources.swapChain;
	tempD3DResources.swapChainVTable = (DWORD_PTR*)tempD3DResources.swapChainVTable[0];
	return true;
}

// Method from unknowncheats, kinda sketchy but also clever
IDXGISwapChain* ScanForSwapChain(uintptr_t ignore = 0) {
	constexpr auto maxAddress = 0x7FFFFFFEFFFF;
	MEMORY_BASIC_INFORMATION64 mbi = { 0 };

	for (uintptr_t memptr = 0x10000; memptr < maxAddress; memptr = mbi.BaseAddress + mbi.RegionSize) {
		if (!VirtualQuery(
			reinterpret_cast<LPCVOID>(memptr),
			reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi),
			sizeof(MEMORY_BASIC_INFORMATION)))
		{
			continue;
		}

		if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS || mbi.Protect & PAGE_GUARD)
			continue;

		DWORD_PTR vtableAddr;
		auto len = mbi.BaseAddress + mbi.RegionSize;
		for (uintptr_t current = mbi.BaseAddress; current < len; ++current) {
			__try {
				vtableAddr = *(DWORD_PTR*)current;
			} __except (1) {
				continue;
			}

			if (vtableAddr == (DWORD_PTR)tempD3DResources.swapChainVTable) {
				if (current == (DWORD_PTR)tempD3DResources.swapChain)
					continue;
				if (ignore != 0 && ignore == current)
					continue;

				return (IDXGISwapChain*)current;
			}
		}
	}

	return nullptr;
}

void DebugDrawing::DetourD3D11() {
	assert(CreateTempResources());

	const auto current = ScanForSwapChain();
	assert(current != 0);

	PLH::VFuncSwapHook d3dHook(
		(uint64_t)current,
		{
			{ static_cast<uint16_t>(8), reinterpret_cast<uint64_t>(&DebugDrawing::Present) },
		},
		&origVFuncs_D3D
	);

	if (!d3dHook.hook()) {
		// If we injected renderdoc, we need to keep scanning
		// @Note: this will cause a crash shortly after level load, but that is fine,
		// just snipe the capture button before it happens!
		const auto secondTry = ScanForSwapChain((uintptr_t)current);
		assert(secondTry);

		PLH::VFuncSwapHook d3dHook2(
			(uint64_t)secondTry,
			{
				{ static_cast<uint16_t>(8), reinterpret_cast<uint64_t>(&DebugDrawing::Present) },
			},
			&origVFuncs_D3D
		);

		if (!d3dHook2.hook()) {
			_ERROR("Failed to place detour on target virtual function, this error is fatal.");
			FatalError(L"Failed to place detour on target virtual function, this error is fatal.");
		}
	}
}
#pragma endregion

DebugDrawing::CommandQueue* DebugDrawing::GetCommandQueue() {
	return &commandQueue;
}

bool DebugDrawing::DrawingEnabled() { return drawingEnabled;  }
void DebugDrawing::SetDrawingEnabled(bool enable) { drawingEnabled = enable; }

void DebugDrawing::Flush() {
	std::lock_guard<std::mutex> lock(commandQueue.lock);
	if (drawingEnabled) {
		for (const auto& cmd : commandQueue.queue) {
			switch (cmd->type) {
				case CommandType::DrawLine:
				{
					drawers.line->Submit(obj, *reinterpret_cast<DrawLine*>(cmd.get()));
					break;
				}
				case CommandType::DrawBox:
				{
					drawers.box->Submit(obj, *reinterpret_cast<DrawBox*>(cmd.get()));
					break;
				}
				default:
					break;
			}
		}
	}
	commandQueue.queue.clear();

	const auto state = GetAsyncKeyState(VK_INSERT);
	if (state && !drawTogglePressed) {
		drawTogglePressed = true;
		drawingEnabled = !drawingEnabled;
	} else if (!state) {
		drawTogglePressed = false;
	}
}

void DebugDrawing::CreateDrawers() {
	drawers.line = std::make_unique<DebugDrawing::LineDrawer>(obj);
	drawers.line->CreateShaders(obj);

	drawers.box = std::make_unique<DebugDrawing::BoxDrawer>(obj);
	drawers.box->CreateShaders(obj);
}

void DebugDrawing::CreateBuffer(size_t size, D3D11_BIND_FLAG binding, ID3D11Buffer*& buffer) {
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = static_cast<UINT>(size);
	desc.BindFlags = binding;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	const auto code = obj.device->CreateBuffer(&desc, nullptr, &buffer);
	assert(SUCCEEDED(code));
	assert(buffer != nullptr);
}

#pragma region LineDrawer
DebugDrawing::LineDrawer::LineDrawer(const D3DObjects& obj) {
	CreateBuffer(sizeof(float) * 16, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, vertexBuffer);
}

bool DebugDrawing::LineDrawer::CreateLayout(const D3DObjects& obj, ID3DBlob* shader) {
	D3D11_INPUT_ELEMENT_DESC desc[] = {
		{ "POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	const auto layoutCode = obj.device->CreateInputLayout(desc, ARRAYSIZE(desc),
		shader->GetBufferPointer(), shader->GetBufferSize(), &inputLayout);

	return SUCCEEDED(layoutCode);
}

bool DebugDrawing::LineDrawer::CreateShaders(const D3DObjects& obj) {
	vertexShader = std::make_unique<Shader>(drawLineVS, ShaderType::VERTEX);
	pixelShader = std::make_unique<Shader>(drawLinePS, ShaderType::FRAGMENT);

	auto vertexBlob = vertexShader->Compile();
	auto fragmentBlob = pixelShader->Compile();
	assert(vertexBlob != nullptr);
	assert(fragmentBlob != nullptr);

	assert(CreateLayout(obj, vertexBlob));
	assert(vertexShader->Create(obj, vertexBlob));
	assert(pixelShader->Create(obj, fragmentBlob));
	return true;
}

void DebugDrawing::LineDrawer::Submit(const D3DObjects& obj, const DebugDrawing::DrawLine& cmd) {
	UINT stride = sizeof(float) * 8;
	UINT offset = 0;
	obj.context->IASetInputLayout(inputLayout);
	obj.context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	const auto code = obj.context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	assert(SUCCEEDED(code));

	auto data = reinterpret_cast<float*>(mappedBuffer.pData);
	data[0] = cmd.start.point.x;
	data[1] = cmd.start.point.y;
	data[2] = 0.0f;
	data[3] = 1.0f;
	data[4] = cmd.start.color.x;
	data[5] = cmd.start.color.y;
	data[6] = cmd.start.color.z;
	data[7] = 1.0f;

	data[8] = cmd.end.point.x;
	data[9] = cmd.end.point.y;
	data[10] = 0.0f;
	data[11] = 1.0f;
	data[12] = cmd.end.color.x;
	data[13] = cmd.end.color.y;
	data[14] = cmd.end.color.z;
	data[15] = 1.0f;

	obj.context->Unmap(vertexBuffer, 0);
	obj.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	vertexShader->Use(obj);
	pixelShader->Use(obj);
	obj.context->Draw(2, 0);
}
#pragma endregion

#pragma region BoxDrawer
DebugDrawing::BoxDrawer::BoxDrawer(const D3DObjects& obj) {
	constexpr auto lineSize = sizeof(float) * 16;
	CreateBuffer(lineSize * 12, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, vertexBuffer);
}

bool DebugDrawing::BoxDrawer::CreateLayout(const D3DObjects& obj, ID3DBlob* shader) {
	D3D11_INPUT_ELEMENT_DESC desc[] = {
		{ "POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	const auto layoutCode = obj.device->CreateInputLayout(desc, ARRAYSIZE(desc),
		shader->GetBufferPointer(), shader->GetBufferSize(), &inputLayout);

	return SUCCEEDED(layoutCode);
}

bool DebugDrawing::BoxDrawer::CreateShaders(const D3DObjects& obj) {
	vertexShader = std::make_unique<Shader>(drawLineVS, ShaderType::VERTEX);
	pixelShader = std::make_unique<Shader>(drawLinePS, ShaderType::FRAGMENT);

	auto vertexBlob = vertexShader->Compile();
	auto fragmentBlob = pixelShader->Compile();
	assert(vertexBlob != nullptr);
	assert(fragmentBlob != nullptr);

	assert(CreateLayout(obj, vertexBlob));
	assert(vertexShader->Create(obj, vertexBlob));
	assert(pixelShader->Create(obj, fragmentBlob));
	return true;
}

void DebugDrawing::BoxDrawer::Submit(const D3DObjects& obj, const DebugDrawing::DrawBox& cmd) {
	UINT stride = sizeof(float) * 8;
	UINT offset = 0;
	obj.context->IASetInputLayout(inputLayout);
	obj.context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	const auto code = obj.context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	assert(SUCCEEDED(code));

	const auto insertLineSegment = [](float* buffer, size_t startIndex, const glm::vec2& a, const glm::vec2& b,
		const glm::vec3& color)
	{
		buffer[startIndex + 0] = a.x;
		buffer[startIndex + 1] = a.y;
		buffer[startIndex + 2] = 0.0f;
		buffer[startIndex + 3] = 1.0f;
		buffer[startIndex + 4] = color.x;
		buffer[startIndex + 5] = color.y;
		buffer[startIndex + 6] = color.z;
		buffer[startIndex + 7] = 1.0f;

		buffer[startIndex + 8] = b.x;
		buffer[startIndex + 9] = b.y;
		buffer[startIndex + 10] = 0.0f;
		buffer[startIndex + 11] = 1.0f;
		buffer[startIndex + 12] = color.x;
		buffer[startIndex + 13] = color.y;
		buffer[startIndex + 14] = color.z;
		buffer[startIndex + 15] = 1.0f;
		return startIndex + 16;
	};

	const auto blf = cmd.box[0];
	const auto brf = cmd.box[1];
	const auto blb = cmd.box[2];
	const auto brb = cmd.box[3];
	const auto tlf = cmd.box[4];
	const auto trf = cmd.box[5];
	const auto tlb = cmd.box[6];
	const auto trb = cmd.box[7];

	auto data = reinterpret_cast<float*>(mappedBuffer.pData);
	size_t currentIndex = 0;
	currentIndex = insertLineSegment(data, currentIndex, blf, blb, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, blf, brf, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, brf, brb, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, brb, blb, cmd.color);

	currentIndex = insertLineSegment(data, currentIndex, tlf, tlb, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, tlf, trf, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, trf, trb, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, trb, tlb, cmd.color);

	currentIndex = insertLineSegment(data, currentIndex, blf, tlf, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, brf, trf, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, blb, tlb, cmd.color);
	currentIndex = insertLineSegment(data, currentIndex, brb, trb, cmd.color);

	obj.context->Unmap(vertexBuffer, 0);
	obj.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	vertexShader->Use(obj);
	pixelShader->Use(obj);
	obj.context->Draw(24, 0);
}
#pragma endregion

#pragma region ShaderLib
ID3DBlob* DebugDrawing::Shader::Compile() {
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
	ID3DBlob* blob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	auto result = D3DCompile(source.c_str(), source.length(), nullptr, nullptr, nullptr, "main",
		type == ShaderType::VERTEX ? "vs_5_0" : "ps_5_0", compileFlags, 0, &blob, &errorBlob);
	
	if (!SUCCEEDED(result)) {
		if (errorBlob)
			OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
		return nullptr;
	}

	if (errorBlob)
		errorBlob->Release();

	return blob;
}

bool DebugDrawing::Shader::Create(const D3DObjects& obj, ID3DBlob* blob) {
	if (type == ShaderType::VERTEX) {
		auto result = obj.device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(),
			nullptr, &vertex);
		blob->Release();
		return SUCCEEDED(result);
	} else {
		auto result = obj.device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(),
			nullptr, &pixel);
		blob->Release();
		return SUCCEEDED(result);
	}
}

void DebugDrawing::Shader::Use(const D3DObjects& obj) const {
	if (type == ShaderType::VERTEX) {
		obj.context->VSSetShader(vertex, nullptr, 0);
	} else {
		obj.context->PSSetShader(pixel, nullptr, 0);
	}
}
#pragma endregion