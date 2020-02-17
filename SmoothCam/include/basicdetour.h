#pragma once

// A simple wrapper around Microsoft Detours for basic function detouring
class BasicDetour {
	public:
		BasicDetour(void** orig, void* detour) noexcept;

		bool Attach() noexcept;
		void Detach() noexcept;

	private:
		void** fnOrig = nullptr;
		void* fnDetour = nullptr;

		bool attached = false;
};