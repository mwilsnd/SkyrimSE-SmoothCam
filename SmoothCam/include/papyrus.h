#pragma once

namespace PapyrusBindings {
	constexpr auto ScriptClassName = "SmoothCamMCM";

	bool Bind(RE::BSScript::IVirtualMachine* vm);
}