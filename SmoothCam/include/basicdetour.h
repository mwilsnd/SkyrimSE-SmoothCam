#pragma once

// A simple wrapper around Microsoft Detours for basic function detouring
class BasicDetour {
	public:
		BasicDetour(void** orig, void* detour) noexcept;
		~BasicDetour() noexcept;
		BasicDetour(const BasicDetour&) = delete;
		BasicDetour(BasicDetour&&) noexcept = delete;
		BasicDetour& operator=(const BasicDetour&) = delete;
		BasicDetour& operator=(BasicDetour&&) noexcept = delete;

		bool Attach() noexcept;
		void Detach() noexcept;

	private:
		void** fnOrig = nullptr;
		void* fnDetour = nullptr;
		bool attached = false;
};

// Some more sugar
template<typename T>
class TypedDetour {
	public:
		explicit TypedDetour(T orig, T detour) noexcept : fnOrig(orig), fnDetour(detour) {}
		explicit TypedDetour(uintptr_t offsetID, T detour) noexcept : fnDetour(detour) {
			fnOrig = Offsets::Get<T>(offsetID);
			assert(fnOrig);
		}

		~TypedDetour() {
			if (attached) Detach();
		}

		TypedDetour(const TypedDetour&) = delete;
		TypedDetour(TypedDetour&&) noexcept = delete;
		TypedDetour& operator=(const TypedDetour&) = delete;
		TypedDetour& operator=(TypedDetour&&) noexcept = delete;

		bool Attach() noexcept {
			assert(!attached);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			attached = DetourAttach(reinterpret_cast<void**>(&fnOrig), reinterpret_cast<void*>(fnDetour))
				== NO_ERROR;

			if (attached)
				DetourTransactionCommit();
			else
				DetourTransactionAbort();

			return attached;
		}

		void Detach() noexcept {
			assert(attached);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(reinterpret_cast<void**>(&fnOrig), reinterpret_cast<void*>(fnDetour));
			DetourTransactionCommit();
		}

		T GetBase() noexcept {
			return fnOrig;
		}

	private:
		T fnOrig = nullptr;
		T fnDetour = nullptr;
		bool attached = false;
};

// For polyhook too, why not
template<typename T>
class VTableDetour {
	public:
		explicit VTableDetour(T* target) noexcept : target(target) {}
		~VTableDetour() noexcept {
			if (attached)
				Detach();
		}

		VTableDetour(const VTableDetour&) = delete;
		VTableDetour(VTableDetour&&) noexcept = delete;
		VTableDetour& operator=(const VTableDetour&) = delete;
		VTableDetour& operator=(VTableDetour&&) noexcept = delete;

		template<typename U>
		void Add(uint16_t index, U fnDetour) noexcept {
			assert(!attached);
			redirects.insert({ index, reinterpret_cast<uint64_t>(fnDetour) });
		}

		void Add(PLH::VFuncMap&& methods) noexcept {
			assert(!attached);
			redirects = eastl::move(methods);
		}

		template<typename U>
		U GetBase(uint16_t index) noexcept {
			return reinterpret_cast<U>(originals.at(index));
		}

		// @NOTE: Because of the union hack, Attach MUST *always* be called, otherwise you risk attempting to run
		// a destructor on an uninitialized object. Yes, this is nasty.
		bool Attach() noexcept {
			assert(!attached);

			new (&hooks) PLH::VFuncSwapHook(
				reinterpret_cast<uint64_t>(target),
				redirects,
				&originals
			);

			attached = hooks.hook();
			return attached;
		}

		void Detach() noexcept {
			assert(attached);
			hooks.unHook();
			attached = false;
		}

	private:
		T* target = nullptr;
		PLH::VFuncMap redirects;
		PLH::VFuncMap originals;

		// Dumb hack for dumb reasons
		union {
			PLH::VFuncSwapHook hooks;
		};

		bool attached = false;
};

// And hooking multiple polymorphic instances
template<typename T, uint8_t numTypes>
class PolymorphicVTableDetour {
	public:
		explicit PolymorphicVTableDetour() {};
		~PolymorphicVTableDetour() {
			if (attached)
				Detach();
		};

		PolymorphicVTableDetour(const PolymorphicVTableDetour&) = delete;
		PolymorphicVTableDetour(PolymorphicVTableDetour&&) noexcept = delete;
		PolymorphicVTableDetour& operator=(const PolymorphicVTableDetour&) = delete;
		PolymorphicVTableDetour& operator=(PolymorphicVTableDetour&&) noexcept = delete;

		template<typename U>
		void Add(T* target, uint16_t index, U fnDetour) noexcept {
			assert(!attached);
			for (auto i = 0; i < targets.size(); ++i) {
				if (targets[i] == target) {
					redirects[i].insert({ index, reinterpret_cast<uint64_t>(fnDetour) });
					return;
				}
			}

			targets.push_back(target);
			redirects.push_back({ { index, reinterpret_cast<uint64_t>(fnDetour) } });
			originals.push_back();
		}

		template<typename U>
		U GetBase(T* target, uint16_t index) noexcept {
			for (auto i = 0; i < targets.size(); ++i)
				if (targets[i] == target)
					return reinterpret_cast<U>(originals[i].at(index));
			return nullptr;
		}

		bool Attach() noexcept {
			assert(!attached);
			for (auto i = 0; i < targets.size(); ++i) {
				hooks.push_back_uninitialized();
				new (&hooks.back()) PLH::VFuncSwapHook(
					reinterpret_cast<uint64_t>(targets[i]),
					redirects[i],
					&originals[i]
				);

				if (!hooks.back().hook())
					return false;
			}
			attached = true;
			return true;
		}

		void Detach() noexcept {
			assert(attached);
			for (auto& it : hooks)
				it.unHook();
			attached = false;
		}

	private:
		eastl::fixed_vector<T*, numTypes, false> targets;
		eastl::fixed_vector<PLH::VFuncMap, numTypes, false> redirects;
		eastl::fixed_vector<PLH::VFuncMap, numTypes, false> originals;
		eastl::fixed_vector<PLH::VFuncSwapHook, numTypes, false> hooks;

		bool attached = false;
};