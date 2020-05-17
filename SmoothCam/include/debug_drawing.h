#pragma once
#include <d3d11.h>

namespace DebugDrawing {
	class Shader;
	class Command;

	typedef struct {
		std::mutex lock;
		std::vector<std::unique_ptr<DebugDrawing::Command>> queue;
	} CommandQueue;
	CommandQueue* GetCommandQueue();
	bool DrawingEnabled();
	void SetDrawingEnabled(bool enable);

	typedef struct {
		IDXGISwapChain* swapChain;
		ID3D11Device* device;
		ID3D11DeviceContext* context;
	} D3DObjects;

	typedef HRESULT(*D3D11Present)(IDXGISwapChain*, UINT, UINT);
	HRESULT Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
	bool CreateTempResources();
	void DetourD3D11();

	enum class CommandType : uint8_t {
		None,
		DrawLine,
		DrawBox
	};

	class Command {
		public:
			CommandType type = CommandType::None;
	};

	struct LineVertex {
		glm::vec2 point;
		glm::vec3 color;
	};

	class DrawLine : public Command {
		public:
			DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color = { 1.0f, 0.0f, 0.0f }) {
				type = CommandType::DrawLine;
				this->start.point = start;
				this->start.color = color;
				this->end.point = end;
				this->end.color = color;
			}

		protected:
			LineVertex start;
			LineVertex end;

		friend class LineDrawer;
	};

	class DrawBox : public Command {
		public:
			using BoxPoints = std::array<glm::vec2, 8>;
			DrawBox(BoxPoints& lines, const glm::vec3& color = { 1.0f, 0.0f, 0.0f }) {
				type = CommandType::DrawBox;
				box = lines;
				this->color = color;
			}

		protected:
			BoxPoints box;
			glm::vec3 color;

		friend class BoxDrawer;
	};

	template<typename T>
	void Submit(T& cmd) {
		static_assert(std::is_base_of<DebugDrawing::Command, T>(), "Argument does not extend from DebugDrawing::Command");
		if (!DebugDrawing::DrawingEnabled()) return;

		auto commandQueue = DebugDrawing::GetCommandQueue();
		// Guarding a vector with a mutex like this is super lazy, but this is a debug tool so whatever
		std::lock_guard<std::mutex> lock(commandQueue->lock);
		commandQueue->queue.push_back(std::move(std::make_unique<T>(cmd)));
	}

	void Flush();
	void CreateDrawers();
	void CreateBuffer(size_t size, D3D11_BIND_FLAG binding, ID3D11Buffer*& buffer);

	class LineDrawer {
		public:
			LineDrawer(const D3DObjects& obj);
			bool CreateLayout(const D3DObjects& obj, ID3DBlob* shader);
			bool CreateShaders(const D3DObjects& obj);
			void Submit(const D3DObjects& obj, const DrawLine& cmd);

		private:
			ID3D11InputLayout* inputLayout = nullptr;
			ID3D11Buffer* vertexBuffer = nullptr;
			D3D11_MAPPED_SUBRESOURCE mappedBuffer;
			std::unique_ptr<Shader> vertexShader;
			std::unique_ptr<Shader> pixelShader;
	};

	class BoxDrawer {
		public:
			BoxDrawer(const D3DObjects& obj);
			bool CreateLayout(const D3DObjects& obj, ID3DBlob* shader);
			bool CreateShaders(const D3DObjects& obj);
			void Submit(const D3DObjects& obj, const DrawBox& cmd);

		private:
			ID3D11InputLayout* inputLayout = nullptr;
			ID3D11Buffer* vertexBuffer = nullptr;
			D3D11_MAPPED_SUBRESOURCE mappedBuffer;
			std::unique_ptr<Shader> vertexShader;
			std::unique_ptr<Shader> pixelShader;
	};

	enum class ShaderType {
		VERTEX,
		FRAGMENT
	};

	class Shader {
		public:
			Shader(const std::string& src, ShaderType type) : source(src), type(type) {}
			ID3DBlob* Compile();
			bool Create(const D3DObjects& obj, ID3DBlob* blob);
			void Use(const D3DObjects& obj) const;

		protected:
			std::string source;
			ShaderType type;
			ID3D11VertexShader* vertex = nullptr;
			ID3D11PixelShader* pixel = nullptr;
	};
}