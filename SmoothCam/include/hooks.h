#pragma once

namespace Hooks {
	// Attach primary game-object hooks
	bool DeferredAttach();

	// Attach hooks for game render startup and shutdown, to enable D3D hooks
	// and created object tear-down.
	bool AttachD3D();

	void ApplyPatches();

	using ShutdownCallback = eastl::function<void()>;
	// Register a function to be called during game shutdown
	void RegisterGameShutdownEvent(ShutdownCallback&& cb) noexcept;
}