#pragma once

enum AutoResolveREL { Yes, No };
template<typename T, AutoResolveREL Res = AutoResolveREL::No>
class TypedDetour {
	public:
		TypedDetour(T orig, T detour) noexcept : fnOrig(orig), fnDetour(detour) {}
		TypedDetour(uintptr_t offsetID, T detour) noexcept : fnDetour(detour) {
			if constexpr (Res == AutoResolveREL::Yes) {
				const auto rel = REL::ID(offsetID);
				fnOrig = REL::Relocation<T>(rel).get();
				baseAddr = rel.address();
			} else {
				fnOrig = REL::Relocation<T>(offsetID).get();
				baseAddr = offsetID;
			}

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

		uintptr_t BaseAddr() const noexcept {
			return baseAddr;
		}

	private:
		T fnOrig = nullptr;
		T fnDetour = nullptr;
		uintptr_t baseAddr = 0;
		bool attached = false;
};