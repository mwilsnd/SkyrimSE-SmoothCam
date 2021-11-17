#pragma once

namespace SkyrimSE {
	class MovementControllerNPC {
		public:
			virtual ~MovementControllerNPC();

			virtual void Unk_01();
			virtual void Unk_02();
			virtual RE::IMovementInterface* QueryMovementInterface(const RE::BSFixedString& movementInterface);
	};

	class IMovementSetGoal : public RE::IMovementInterface {
		public:
			virtual ~IMovementSetGoal();
			virtual void Unk_01();
			virtual void Unk_02();
			virtual void ClearPathingRequest();
	};
}