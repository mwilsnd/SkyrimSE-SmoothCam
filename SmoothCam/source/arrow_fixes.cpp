#include "arrow_fixes.h"
#include "game_state.h"
#include "camera.h"
#include "thirdperson.h"
#include "debug/eh.h"

extern eastl::unique_ptr<Camera::Camera> g_theCamera;

#ifdef DEBUG
// Draw the flight path of the last fired projectile for debug help
// INSERT toggles the overlay

static eastl::unique_ptr<Render::LineDrawer> segmentDrawer;
std::mutex segmentLock;
static eastl::vector<eastl::tuple<glm::vec3, glm::vec3>> points;
static Render::LineList segments;
static bool drawOverlay = false;


typedef uintptr_t(*UpdateTraceArrowProjectile)(SkyrimSE::ArrowProjectile*, NiPoint3*, NiPoint3*);
using TickArrowFlightPath = TypedDetour<UpdateTraceArrowProjectile>;
static eastl::unique_ptr<TickArrowFlightPath> detUpdateTraceArrowProjectile;
static uintptr_t mUpdateTraceArrowProjectile(SkyrimSE::ArrowProjectile* arrow, NiPoint3* to, NiPoint3* from) {
	if (arrow->shooter == 0x00100000) {
		std::lock_guard<std::mutex> lock(segmentLock);
		points.push_back(eastl::make_tuple(glm::vec3{ from->x, from->y, from->z }, glm::vec3{ to->x, to->y, to->z }));
	}
	return detUpdateTraceArrowProjectile->GetBase()(arrow, to, from);
}


typedef UInt32(*MaybeSpawnArrow)(uint32_t* arrowHandle, ArrowFixes::LaunchData* launchData,
	uintptr_t param_3, uintptr_t** param_4);
using ArrowSpawnFunc = TypedDetour<MaybeSpawnArrow>;
static eastl::unique_ptr<ArrowSpawnFunc> detArrowSpawn;
static UInt32 mArrowSpawn(uint32_t* arrowHandle, ArrowFixes::LaunchData* launchData,
	uintptr_t param_3, uintptr_t** param_4)
{
	auto ret = detArrowSpawn->GetBase()(arrowHandle, launchData, param_3, param_4);
	NiPointer<TESObjectREFR> ref;
	UInt32 rc = *arrowHandle;
	(*LookupREFRByHandle)(rc, ref);
	auto asArrow = reinterpret_cast<SkyrimSE::ArrowProjectile*>(ref.get());

	if (asArrow->shooter == 0x00100000) {
		std::lock_guard<std::mutex> lock(segmentLock);
		points.clear();
	}

	return ret;		
}

static bool insertWasDown = false;
void ArrowFixes::Draw(Render::D3DContext& ctx) {
	if (GetAsyncKeyState(VK_INSERT) && !insertWasDown) {
		insertWasDown = true;
		drawOverlay = !drawOverlay;
	} else if (!GetAsyncKeyState(VK_INSERT)) {
		insertWasDown = false;
	}

	if (!drawOverlay) return;

	std::lock_guard<std::mutex> lock(segmentLock);
	segments.clear();
	for (const auto& [l1, l2] : points) {
		segments.emplace_back(
			Render::Point(
				Render::ToRenderScale(l1),
				{ 1.0f, 0.0f, 0.0f, 1.0f }
			),
			Render::Point(
				Render::ToRenderScale(l2),
				{ 1.0f, 0.0f, 0.0f, 1.0f }
			)
		);
	}

	segmentDrawer->Submit(segments);
}
#endif


//FUN_14084b430:49866
typedef void(*FactorCameraOffset)(CorrectedPlayerCamera* camera, NiPoint3& pos, bool fac);
using FactorCameraOffsetDetour = TypedDetour<FactorCameraOffset>;
static eastl::unique_ptr<FactorCameraOffsetDetour> detFactorCameraOffset;
static void mFactorCameraOffset(CorrectedPlayerCamera* camera, NiPoint3& pos, bool fac) {
	const auto mdmp = Debug::MiniDumpScope();
	
	// SSE Engine Fixes will call GetEyeVector with factorCameraOffset = true
	// We appear to screw this computation up as a side effect of correcting the interaction crosshair
	// So, yeah. Just fix it here.

	if (fac || Config::GetCurrentConfig()->modDisabled) {
		detFactorCameraOffset->GetBase()(camera, pos, fac);
		return;
	}

	// Only run in states we care about
	const auto ply = *g_thePlayer;
	if (!ply) {
		detFactorCameraOffset->GetBase()(camera, pos, fac);
		return;
	}

	if (!GameState::IsThirdPerson(ply, camera) && !GameState::IsInHorseCamera(ply, camera) && !GameState::IsInDragonCamera(camera)) {
		detFactorCameraOffset->GetBase()(camera, pos, fac);
		return;
	}

	// Just return an offset of zero
	pos = { 0, 0, 0 };
}

typedef void(*UpdateArrowFlightPath)(SkyrimSE::ArrowProjectile* arrow);
using UpdateArrowFlightPathDetour = TypedDetour<UpdateArrowFlightPath>;
static eastl::unique_ptr<UpdateArrowFlightPathDetour> detArrowFlightPath;
static void mUpdateArrowFlightPath(SkyrimSE::ArrowProjectile* arrow) {
	const auto mdmp = Debug::MiniDumpScope();

	if (!g_theCamera || Config::GetCurrentConfig()->modDisabled)
		return detArrowFlightPath->GetBase()(arrow);

	const auto camera = CorrectedPlayerCamera::GetSingleton();
	const auto ply = *g_thePlayer;

	// Only correct our own arrows
	UInt32 ref = arrow->shooter;
	if (ref == *g_invalidRefHandle)
		return detArrowFlightPath->GetBase()(arrow);

	NiPointer<TESObjectREFR> out;
	(*LookupREFRByHandle)(ref, out);

	// On horseback, our camera actually follows the horse and not the player
	// Compare with the player directly
	if (reinterpret_cast<const intptr_t>(out.get()) != reinterpret_cast<const intptr_t>(ply))
		return detArrowFlightPath->GetBase()(arrow);

	// @Note: Don't run our trajectory correction while in a kill move
	// Only run it in the primary states we care about
	const auto state = g_theCamera->GetCurrentCameraState();
	if (state != GameState::CameraState::ThirdPerson && state != GameState::CameraState::ThirdPersonCombat
		&& state != GameState::CameraState::Horseback && state != GameState::CameraState::IronSights)
	{
		return detArrowFlightPath->GetBase()(arrow);
	}

	// Get camera aim rotation
	auto rot = g_theCamera->GetThirdpersonCamera()->GetAimRotation(ply, camera);

	// Compute arrow velocity vector
	typedef float(*GetAFloat)(SkyrimSE::ArrowProjectile*);
	const auto s2 = Offsets::Get<GetAFloat>(42537)(arrow); // Does the same thing as equippedWeapon->gameData.speed
	const auto power = Offsets::Get<GetAFloat>(42536)(arrow); // Scalar, 0-1 how long you held back the arrow (arrow->unk188)

	// Not sure what this is looking for, but do it anyways
	if ((~(byte)(arrow->flags >> 0x1f) & 1) != 0) {
	// Assume otherwise is a magic projectile
		if (s2 != 1.0f) {
			static auto arrowTilt = (*g_iniSettingCollection)->Get("f3PArrowTiltUpAngle:Combat");
			// Add tilt angle
			if (GameState::IsUsingBow(ply) || GameState::IsUsingCrossbow(ply)) {
				float tilt = 2.5f;
				if (arrowTilt)
					tilt = arrowTilt->data.f32;
				rot.x -= glm::radians(tilt);
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

	// Appears to look for the shooter, then add the shooter's velocity to the arrow
	typedef void(*LookupFun)(uint32_t*, TESObjectREFR**);
	static auto lookupFunc = Offsets::Get<LookupFun>(12204);

	uint32_t shooterID = arrow->shooter;
	TESObjectREFR* shooterRef = nullptr;
	lookupFunc(&shooterID, &shooterRef);

	const TESObjectREFR* DAT_142eff7d8 = *Offsets::Get<TESObjectREFR**>(514905);
	const auto unkData = Offsets::Get<ArrowFixes::UnkData*>(514725);
	if (((shooterRef != nullptr) && (shooterRef == DAT_142eff7d8)) && (unkData->unk4 != 4))
	{
		NiPoint3 shooterVelocity;
		typedef void(__thiscall TESObjectREFR::* GetLinearVelocity)(NiPoint3&); // GetLinearVelocity, So says CommonLib
		(ply->*reinterpret_cast<GetLinearVelocity>(&TESObjectREFR::Unk_86))(shooterVelocity);
		arrow->velocityVector.x = shooterVelocity.x + arrow->velocityVector.x;
		arrow->velocityVector.y = shooterVelocity.y + arrow->velocityVector.y;
	}

	// Like other handle refcounters, arg1 = 0, release rc if arg2 != nullptr
	shooterID = 0;
	lookupFunc(&shooterID, &shooterRef);
}

bool ArrowFixes::Attach() {
	detFactorCameraOffset = eastl::make_unique<FactorCameraOffsetDetour>(49866, mFactorCameraOffset);
	if (!detFactorCameraOffset->Attach()) {
		_ERROR("Failed to place detour on target function(49,866), this error is fatal.");
		FatalError(L"Failed to place detour on target function(49,866), this error is fatal.");
	}

	detArrowFlightPath = eastl::make_unique<UpdateArrowFlightPathDetour>(42998, mUpdateArrowFlightPath);
	if (!detArrowFlightPath->Attach()) {
		_ERROR("Failed to place detour on target function(42,998), this error is fatal.");
		FatalError(L"Failed to place detour on target function(42,998), this error is fatal.");
	}

#ifdef DEBUG
	detUpdateTraceArrowProjectile = eastl::make_unique<TickArrowFlightPath>(43008, mUpdateTraceArrowProjectile);
	if (!detUpdateTraceArrowProjectile->Attach()) {
		_ERROR("Failed to place detour on target function(43,008), this error is fatal.");
		FatalError(L"Failed to place detour on target function(43,008), this error is fatal.");
	}

	detArrowSpawn = eastl::make_unique<ArrowSpawnFunc>(42928, mArrowSpawn);
	if (!detArrowSpawn->Attach()) {
		_ERROR("Failed to place detour on target function(42,928), this error is fatal.");
		FatalError(L"Failed to place detour on target function(42,928), this error is fatal.");
	}

	if (Render::HasContext()) {
		segmentDrawer = eastl::make_unique<Render::LineDrawer>(Render::GetContext());
	}
#endif

	return true;
}

#ifdef DEBUG
void ArrowFixes::Shutdown() {
	std::lock_guard<std::mutex> lock(segmentLock);
	segmentDrawer.reset();
}
#endif