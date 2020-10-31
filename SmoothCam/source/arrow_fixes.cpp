#include "arrow_fixes.h"
#include "game_state.h"
#include "camera.h"

extern std::shared_ptr<Camera::SmoothCamera> g_theCamera;

//FUN_14084b430:49866
typedef void(*FactorCameraOffset)(CorrectedPlayerCamera* camera, NiPoint3& pos, bool fac);
static FactorCameraOffset fnFactorCameraOffset;
static std::unique_ptr<BasicDetour> detFactorCameraOffset;
static void mFactorCameraOffset(CorrectedPlayerCamera* camera, NiPoint3& pos, bool fac) {
	// SSE Engine Fixes will call GetEyeVector with factorCameraOffset = true
	// We appear to screw this computation up as a side effect of correcting the interaction crosshair
	// So, yeah. Just fix it here.

	if (fac) {
		fnFactorCameraOffset(camera, pos, fac);
		return;
	}

	// Only run in states we care about
	static const auto ply = *g_thePlayer;
	if (!GameState::IsThirdPerson(ply, camera) && !GameState::IsInHorseCamera(ply, camera) && !GameState::IsInDragonCamera(camera)) {
		fnFactorCameraOffset(camera, pos, fac);
		return;
	}

	// Just return an offset of zero
	pos = { 0, 0, 0 };
}

typedef void(*UpdateArrowFlightPath)(SkyrimSE::ArrowProjectile* arrow);
static UpdateArrowFlightPath fnUpdateArrowFlightPath;
static std::unique_ptr<BasicDetour> detArrowFlightPath;
static void mUpdateArrowFlightPath(SkyrimSE::ArrowProjectile* arrow) {
	if (!g_theCamera)
		return fnUpdateArrowFlightPath(arrow);

	const auto camera = CorrectedPlayerCamera::GetSingleton();
	const auto ply = *g_thePlayer;

	// Only correct our own arrows
	UInt32 ref = arrow->shooter;
	NiPointer<TESObjectREFR> out;
	(*LookupREFRByHandle)(ref, out);

	// On horseback, our camera actually follows the horse and not the player
	// Compare with the player directly
	// @TODO: When refactoring the camera to follow the active input ref, change this check too
	if (reinterpret_cast<const intptr_t>(out.get()) != reinterpret_cast<const intptr_t>(ply))
		return fnUpdateArrowFlightPath(arrow);

	// @Note: Don't run our trajectory correction while in a kill move
	// Only run it in the primary states we care about
	const auto state = g_theCamera->GetCurrentCameraState();
	if (state != GameState::CameraState::ThirdPerson && state != GameState::CameraState::ThirdPersonCombat
		&& state != GameState::CameraState::Horseback && state != GameState::CameraState::IronSights)
	{
		return fnUpdateArrowFlightPath(arrow);
	}

	const auto tps = reinterpret_cast<const CorrectedThirdPersonState*>(
		camera->cameraStates[CorrectedPlayerCamera::kCameraState_ThirdPerson2]
	);
	if (!tps) return
		fnUpdateArrowFlightPath(arrow);

	// Get camera aim rotation
	auto rot = g_theCamera->GetAimRotation(ply, camera);
	// Compute arrow velocity vector
	typedef float(*GetAFloat)(SkyrimSE::ArrowProjectile*);

	// Doesn't look like gravity - might be a scalar applied to gravity?
	// Does the same thing as equippedWeapon->gameData.speed
	const auto s2 = Offsets::Get<GetAFloat>(42537)(arrow);
	// Scalar, 0-1 how long you held back the arrow
	const auto power = Offsets::Get<GetAFloat>(42536)(arrow);

	// Not sure what this is looking for, but do it anyways
	if ((~(byte)(arrow->flags >> 0x1f) & 1) != 0) {
		// Assume otherwise is a magic projectile
		if (s2 != 1.0f) {
			// Add tilt angle
			if (GameState::IsUsingCrossbow(*g_thePlayer)) {
				rot.x -= glm::radians(Config::GetGameConfig()->f3PBoltTiltUpAngle);
			} else if (GameState::IsUsingBow(*g_thePlayer)) {
				rot.x -= glm::radians(Config::GetGameConfig()->f3PArrowTiltUpAngle);
			}
		}
	} else {
		rot.x = arrow->rot.x;
		rot.y = arrow->rot.z;
	}

	const auto projectileForm = reinterpret_cast<const BGSProjectile*>(arrow->baseForm);
	const auto speed = projectileForm->data.speed;
	const auto arrowFireSpeed = speed * power;
	const auto alwaysOne = arrow->unk18C; // At least, as far as I've seen
	const auto velScalar = s2 * arrowFireSpeed * alwaysOne;

	rot.x *= -1.0f;
	const auto pitchCos = glm::cos(rot.x);
	const auto pitchSin = glm::sin(rot.x);
	const auto yawCos = glm::cos(rot.y);
	const auto yawSin = glm::sin(rot.y);

	const auto pitchCosVel = pitchCos * velScalar;
	const auto pitchSinVel = pitchSin * velScalar;
	arrow->velocityVector = {
		pitchCosVel * yawSin,
		pitchCosVel * yawCos,
		pitchSinVel
	};

	// Still don't know what this all is doing
	// Takes a lock, gets some globals, adds an x,y offset to velocity
	typedef void(*LookupFun)(uint32_t*, uintptr_t*);
	static auto lookupFunc = Offsets::Get<LookupFun>(12204);

	uint32_t local_res8 = arrow->shooter;
	uintptr_t local_res10 = 0;
	lookupFunc(&local_res8, &local_res10); // FUN_1401329d0

	auto uVar3 = local_res10;

	const uintptr_t DAT_142eff7d8 = Offsets::Get<uintptr_t>(514905);
	const uintptr_t DAT_142ec5c60 = Offsets::Get<uintptr_t>(514725);
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
	lookupFunc(&local_res8, &local_res10);
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
			_ERROR("Failed to place detour on target function(49,866), this error is fatal.");
			FatalError(L"Failed to place detour on target function(49,866), this error is fatal.");
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
			_ERROR("Failed to place detour on target function(42,998), this error is fatal.");
			FatalError(L"Failed to place detour on target function(42,998), this error is fatal.");
		}
	}

	return true;
}