#include "arrow_fixes.h"
#include "game_state.h"

#ifdef DEBUG_DRAWING
SkyrimSE::ArrowProjectile* current;
std::mutex segmentLock;
std::vector<std::tuple<glm::vec3, glm::vec3>> segments;

void ArrowFixes::Draw() {
	std::lock_guard<std::mutex> lock(segmentLock);
	for (auto& cmd : segments) {
		DebugDrawing::Submit(DebugDrawing::DrawLine(
			mmath::PointToScreen(std::get<0>(cmd)),
			mmath::PointToScreen(std::get<1>(cmd))
		));
	}
}

typedef UInt32(*MaybeSpawnArrow)(uint32_t* arrowHandle, ArrowFixes::LaunchData* launchData,
	uintptr_t param_3, uintptr_t** param_4);
MaybeSpawnArrow arrOrig;
std::unique_ptr<BasicDetour> detMaybeArrow;
UInt32 mMaybeSpawnArrow(uint32_t* arrowHandle, ArrowFixes::LaunchData* launchData,
	uintptr_t param_3, uintptr_t** param_4)
{
	auto ret = arrOrig(arrowHandle, launchData, param_3, param_4);
	NiPointer<TESObjectREFR> ref;
	UInt32 rc = *arrowHandle;
	(*LookupREFRByHandle)(rc, ref);
	auto asArrow = reinterpret_cast<SkyrimSE::ArrowProjectile*>(ref.get());

	if (asArrow->shooter == 0x00100000) {
		std::lock_guard<std::mutex> lock(segmentLock);
		segments.clear();
	}
	return ret;
}

typedef uintptr_t(*UpdateTraceArrowProjectile)(SkyrimSE::ArrowProjectile*, NiPoint3&, NiPoint3&);
UpdateTraceArrowProjectile fnUpdateTraceArrowProjectile;
std::unique_ptr<BasicDetour> detUpdateTraceArrowProjectile;
uintptr_t mUpdateTraceArrowProjectile(SkyrimSE::ArrowProjectile* arrow, NiPoint3& to, NiPoint3& from) {
	if (arrow->shooter == 0x00100000) {
		std::lock_guard<std::mutex> lock(segmentLock);
		segments.push_back(std::make_tuple(glm::vec3{ from.x, from.y, from.z }, glm::vec3{ to.x, to.y, to.z }));
	}
	auto ret = fnUpdateTraceArrowProjectile(arrow, to, from);
	return ret;
}
#endif

//FUN_14084b430:49866
typedef void(*FactorCameraOffset)(CorrectedPlayerCamera* camera, NiPoint3& pos, bool fac);
FactorCameraOffset fnFactorCameraOffset;
std::unique_ptr<BasicDetour> detFactorCameraOffset;
void mFactorCameraOffset(CorrectedPlayerCamera* camera, NiPoint3& pos, bool fac) {
	if (fac) {
		fnFactorCameraOffset(camera, pos, fac);
		return;
	}

	auto tps = reinterpret_cast<CorrectedThirdPersonState*>(camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]);
	if (!tps) {
		fnFactorCameraOffset(camera, pos, fac);
		return;
	}

	NiQuaternion quat;
	NiMatrix33 mat;

	tps->UpdateRotation();
	typedef void(__thiscall CorrectedThirdPersonState::* GetRotation)(NiQuaternion&);
	(tps->*reinterpret_cast<GetRotation>(&CorrectedThirdPersonState::Unk_04))(quat);

	//1cfa50:makeMatrix33Qua
	typedef void(*makeMatrix33Qua)(NiQuaternion& q, NiMatrix33& m);
	Offsets::Get<makeMatrix33Qua>(15612)(quat, mat);

	NiPoint3 offsetActual;

	// SSE Engine Fixes will call GetEyeVector with factorCameraOffset = true
	// We appear to screw this computation up as a side effect of correcting the interaction crosshair
	// So, yeah. Just fix it here.
	if (GameState::IsThirdPerson(camera) || GameState::IsInHorseCamera(camera) || GameState::IsInDragonCamera(camera))
		offsetActual = { 0, 0, 0 };
	else
		offsetActual = tps->offsetVector;
	pos = mat * offsetActual;
}

typedef void(*UpdateArrowFlightPath)(SkyrimSE::ArrowProjectile* arrow);
UpdateArrowFlightPath fnUpdateArrowFlightPath;
std::unique_ptr<BasicDetour> detArrowFlightPath;
void mUpdateArrowFlightPath(SkyrimSE::ArrowProjectile* arrow) {
	auto camera = CorrectedPlayerCamera::GetSingleton();
	if (arrow->shooter != camera->playerRef || camera->cameraState == camera->cameraStates[CorrectedPlayerCamera::kCameraState_FirstPerson])
		return fnUpdateArrowFlightPath(arrow);

	//578 - 1407320a0 - 42536
	//580 - 1407320c0 - 42537
	typedef float(*GetAFloat)(SkyrimSE::ArrowProjectile*);
	auto gravity = Offsets::Get<GetAFloat>(42537)(arrow);
	
	auto projectileForm = reinterpret_cast<BGSProjectile*>(arrow->baseForm);

	float arrPitch;
	float arrRotation;
	if ((~(byte)(arrow->flags >> 0x1f) & 1) != 0) {
		const auto mat = camera->cameraNode->m_localTransform.rot;

		if (gravity == 1.0f) // assume this is a magic projectile
			arrPitch = glm::asin(glm::clamp(-mat.data[2][1], -1.0f, 1.0f));
		else {
			if (GameState::IsUsingCrossbow(*g_thePlayer)) {
				arrPitch = glm::asin(glm::clamp(-mat.data[2][1], -1.0f, 1.0f)) - glm::radians(Config::GetGameConfig()->f3PBoltTiltUpAngle);
			} else if (GameState::IsUsingBow(*g_thePlayer)) {
				arrPitch = glm::asin(glm::clamp(-mat.data[2][1], -1.0f, 1.0f)) - glm::radians(Config::GetGameConfig()->f3PArrowTiltUpAngle);
			} else {
				arrPitch = glm::asin(glm::clamp(-mat.data[2][1], -1.0f, 1.0f));
			}
		}

		arrRotation = camera->lookYaw;
	} else {
		arrPitch = arrow->rot.x;
		arrRotation = arrow->rot.z;
	}

	float _X = projectileForm->data.speed;

	//arrow->unk175(); // fVar5 = (float)(**(code**)(*(longlong*)param_1 + 0x578))();
	float fVar5 = Offsets::Get<GetAFloat>(42536)(arrow);
	fVar5 = fVar5 * _X;

	//arrow->unk176(); //_X = (float)(**(code **)(*(longlong *)param_1 + 0x580))(param_1);
	//_X = Offsets::Get<GetAFloat>(42537)(arrow);
	float fVar6 = gravity * fVar5 * arrow->unk18C;

	//(float)((uint32_t)(arrow->rot).x ^ 0x80000000);
	_X = arrPitch/*arrow->rot.x*/ * -1.0f;
	fVar5 = cosf(_X);
	fVar5 = fVar5 * fVar6;
	float fVar4 = sinf(_X);
	fVar4 = fVar4 * fVar6;
	_X = arrRotation; // arrow->rot.z;
	fVar6 = sinf(_X);
	_X = cosf(_X);

	arrow->velocityVector.x = fVar5 * fVar6;
	arrow->velocityVector.y = fVar5 * _X;
	arrow->velocityVector.z = fVar4;

	typedef void(*LookupFun)(uint32_t*, uintptr_t*);

	uint32_t local_res8 = arrow->shooter;
	uintptr_t local_res10 = 0;
	Offsets::Get<LookupFun>(12204)(&local_res8, &local_res10); // FUN_1401329d0
	auto uVar3 = local_res10;

	uintptr_t DAT_142eff7d8 = Offsets::Get<uintptr_t>(514905);
	uintptr_t DAT_142ec5c60 = Offsets::Get<uintptr_t>(514725);
	if (((local_res10 != 0) && (local_res10 == DAT_142eff7d8)) &&
		(*(int*)(DAT_142ec5c60 + 0x20) != 4))
	{
		NiPoint3 local_70;
		//(**(code **)(*ThePlayer + 0x430))(ThePlayer, &local_70);
		typedef void(__thiscall PlayerCharacter::* Unk)(NiPoint3&);
		(*g_thePlayer->*reinterpret_cast<Unk>(&PlayerCharacter::Unk_86))(local_70);
		arrow->velocityVector.x = local_70.x + arrow->velocityVector.x;
		arrow->velocityVector.y = local_70.y + arrow->velocityVector.y;
	}

	// Like other handle refcounters, arg1 = 0, release rc if arg2 != nullptr
	local_res8 = 0;
	Offsets::Get<LookupFun>(12204)(&local_res8, &local_res10);
}

bool ArrowFixes::Attach() {
	{
		//FUN_14084b430:FactorCameraOffset:GetEyeVector
		fnFactorCameraOffset = Offsets::Get<FactorCameraOffset>(49866);
		detFactorCameraOffset = std::make_unique<BasicDetour>(
			reinterpret_cast<void**>(&fnFactorCameraOffset),
			mFactorCameraOffset
		);

		if (!detFactorCameraOffset->Attach()) {
			_ERROR("Failed to place detour on target function, this error is fatal.");
			FatalError(L"Failed to place detour on target function, this error is fatal.");
		}
	}

	{
		//140750150::UpdateArrowFlightPath
		fnUpdateArrowFlightPath = Offsets::Get<UpdateArrowFlightPath>(42998);
		detArrowFlightPath = std::make_unique<BasicDetour>(
			reinterpret_cast<void**>(&fnUpdateArrowFlightPath),
			mUpdateArrowFlightPath
		);

		if (!detArrowFlightPath->Attach()) {
			_ERROR("Failed to place detour on target function, this error is fatal.");
			FatalError(L"Failed to place detour on target function, this error is fatal.");
		}
	}

#ifdef DEBUG_DRAWING
	{
		arrOrig = Offsets::Get<MaybeSpawnArrow>(42928);
		detMaybeArrow = std::make_unique<BasicDetour>(
			reinterpret_cast<void**>(&arrOrig),
			mMaybeSpawnArrow
		);

		if (!detMaybeArrow->Attach()) {
			_ERROR("Failed to place detour on target function, this error is fatal.");
			FatalError(L"Failed to place detour on target function, this error is fatal.");
		}
	}

	{
		//140751430::UpdateTraceArrowProjectile
		fnUpdateTraceArrowProjectile = Offsets::Get<UpdateTraceArrowProjectile>(43008);
		detUpdateTraceArrowProjectile = std::make_unique<BasicDetour>(
			reinterpret_cast<void**>(&fnUpdateTraceArrowProjectile),
			mUpdateTraceArrowProjectile
			);

		if (!detUpdateTraceArrowProjectile->Attach()) {
			_ERROR("Failed to place detour on target function, this error is fatal.");
			FatalError(L"Failed to place detour on target function, this error is fatal.");
		}
	}
#endif

	return true;
}