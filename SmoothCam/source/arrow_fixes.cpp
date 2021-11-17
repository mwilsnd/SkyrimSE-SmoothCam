#include "arrow_fixes.h"
#include "camera.h"
#include "thirdperson.h"
#include "detours.h"
#ifdef DEBUG
#include "render/line_drawer.h"
#endif
#include "debug/eh.h"

extern eastl::unique_ptr<Camera::Camera> g_theCamera;
extern Offsets* g_Offsets;

#ifdef DEBUG
// Draw the flight path of the last fired projectile for debug help
// INSERT toggles the overlay

static eastl::unique_ptr<Render::LineDrawer> segmentDrawer;
std::mutex segmentLock;
static eastl::vector<eastl::tuple<glm::vec3, glm::vec3>> points;
static Render::LineList segments;
static bool drawOverlay = false;


typedef uintptr_t(*UpdateTraceArrowProjectile)(RE::Projectile*, RE::NiPoint3*, RE::NiPoint3*);
using TickArrowFlightPath = TypedDetour<UpdateTraceArrowProjectile>;
static eastl::unique_ptr<TickArrowFlightPath> detUpdateTraceArrowProjectile;
static uintptr_t mUpdateTraceArrowProjectile(RE::Projectile* arrow, RE::NiPoint3* to, RE::NiPoint3* from) {
	if (arrow->shooter.native_handle() == 0x00100000) {
		std::lock_guard<std::mutex> lock(segmentLock);
		points.push_back(eastl::make_tuple(glm::vec3{ from->x, from->y, from->z }, glm::vec3{ to->x, to->y, to->z }));
	}
	return detUpdateTraceArrowProjectile->GetBase()(arrow, to, from);
}


typedef uint32_t(*MaybeSpawnArrow)(uint32_t* arrowHandle, ArrowFixes::LaunchData* launchData,
	uintptr_t param_3, uintptr_t** param_4);
using ArrowSpawnFunc = TypedDetour<MaybeSpawnArrow>;
static eastl::unique_ptr<ArrowSpawnFunc> detArrowSpawn;
static uint32_t mArrowSpawn(uint32_t* arrowHandle, ArrowFixes::LaunchData* launchData,
	uintptr_t param_3, uintptr_t** param_4)
{
	const auto ret = detArrowSpawn->GetBase()(arrowHandle, launchData, param_3, param_4);
	RE::NiPointer<RE::TESObjectREFR> ref;
	RE::LookupReferenceByHandle(*arrowHandle, ref);

	auto asArrow = reinterpret_cast<RE::ArrowProjectile*>(ref.get());
	if (asArrow->shooter.native_handle() == 0x00100000) {
		std::lock_guard<std::mutex> lock(segmentLock);
		points.clear();
	}

	return ret;		
}

static bool insertWasDown = false;
void ArrowFixes::Draw(Render::D3DContext&) {
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

typedef void(*UpdateArrowFlightPath)(RE::Projectile* arrow);
using UpdateArrowFlightPathDetour = TypedDetour<UpdateArrowFlightPath>;
static eastl::unique_ptr<UpdateArrowFlightPathDetour> detArrowFlightPath;
static void mUpdateArrowFlightPath(RE::Projectile* arrow) {
	const auto mdmp = Debug::MiniDumpScope();
	const auto ply = RE::PlayerCharacter::GetSingleton();

	if (!g_theCamera || Config::GetCurrentConfig()->modDisabled ||
		Messaging::SmoothCamInterface::GetInstance()->IsCameraTaken())
		return detArrowFlightPath->GetBase()(arrow);

	// Only correct our own arrows
	if (!arrow->shooter || arrow->shooter.get().get() != ply)
		return detArrowFlightPath->GetBase()(arrow);

	// Check for a valid weapon and ammo source - easy way to check for arrows
	if (!arrow->weaponSource || !arrow->ammoSource)
		return detArrowFlightPath->GetBase()(arrow);
	
	// On horseback, our camera actually follows the horse and not the player
	// Compare with the player directly
	// @Note: Don't run our trajectory correction while in a kill move
	// Only run it in the primary states we care about
	const auto state = g_theCamera->GetCurrentCameraState();
	if (state != GameState::CameraState::ThirdPerson && state != GameState::CameraState::ThirdPersonCombat
		&& state != GameState::CameraState::Horseback && state != GameState::CameraState::IronSights)
	{
		return detArrowFlightPath->GetBase()(arrow);
	}

	// Get camera aim rotation
	auto rot = g_theCamera->GetThirdpersonCamera()->GetAimRotation(ply, RE::PlayerCamera::GetSingleton());

	// Compute arrow velocity vector
	typedef float(__thiscall *GetAFloat)(RE::Projectile*);
	const auto s2 = REL::Relocation<GetAFloat>(g_Offsets->S2)(arrow); // Does the same thing as equippedWeapon->gameData.speed
	const auto power = REL::Relocation<GetAFloat>(g_Offsets->Power)(arrow); // Scalar, 0-1 how long you held back the arrow (arrow->unk188)
	
	// Not sure what this is looking for, but do it anyways
	if ((~(byte)(arrow->flags >> 0x1f) & 1) != 0) {
		// @Note: s2 being 1 does NOT mean this is magic
		if (skyrim_cast<RE::ArrowProjectile*>(arrow)) {
			static auto arrowTilt = RE::INISettingCollection::GetSingleton()->GetSetting("f3PArrowTiltUpAngle:Combat");
			// Add tilt angle
			if (GameState::IsUsingBow(ply) || GameState::IsUsingCrossbow(ply)) {
				float tilt = 2.5f;
				if (arrowTilt)
					tilt = arrowTilt->GetFloat();
				rot.x -= glm::radians(tilt);
			}
		}
	} else {
		rot.x = arrow->GetAngleX();
		rot.y = arrow->GetAngleZ();
	}

	const auto projectileForm = reinterpret_cast<const RE::BGSProjectile*>(arrow->GetBaseObject());
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

	// projectile->velocityVector
	*reinterpret_cast<RE::NiPoint3*>(&arrow->unk0FC) = {
		pitchCosVel * yawSin,
		pitchCosVel * yawCos,
		pitchSinVel
	};

	// Appears to look for the shooter, then add the shooter's velocity to the arrow
	typedef void(*LookupFun)(uint32_t*, RE::TESObjectREFR**);
	static auto lookupFunc = REL::Relocation<LookupFun>(g_Offsets->RWLocker);

	uint32_t shooterID = arrow->shooter.native_handle();
	RE::TESObjectREFR* shooterRef = nullptr;
	lookupFunc(&shooterID, &shooterRef);

	RE::TESObjectREFR** DAT_142eff7d8 = REL::Relocation<RE::TESObjectREFR**>(g_Offsets->DAT_142eff7d8).get();
	const auto unkData = REL::Relocation<ArrowFixes::UnkData*>(g_Offsets->UnkData);
	if (DAT_142eff7d8 && ((shooterRef != nullptr) && (shooterRef == *DAT_142eff7d8)) && (unkData->unk4 != 4))
	{
		RE::NiPoint3 shooterVelocity;
		ply->GetLinearVelocity(shooterVelocity);
		// projectile->velocityVector
		arrow->unk0FC = shooterVelocity.x + arrow->unk0FC;
		arrow->unk100 = shooterVelocity.y + arrow->unk100;
	}

	// Like other handle refcounters, arg1 = 0, release rc if arg2 != nullptr
	shooterID = 0;
	lookupFunc(&shooterID, &shooterRef);
}

bool ArrowFixes::Attach() {
	detArrowFlightPath = eastl::make_unique<UpdateArrowFlightPathDetour>(g_Offsets->UpdateFlightPath, mUpdateArrowFlightPath);
	if (!detArrowFlightPath->Attach())
		FatalError(L"Failed to place detour on target function(ArrowFixes::UpdateFlightPath), this error is fatal.");

#ifdef DEBUG
	detUpdateTraceArrowProjectile = eastl::make_unique<TickArrowFlightPath>(g_Offsets->DebugTraceProjectile, mUpdateTraceArrowProjectile);
	if (!detUpdateTraceArrowProjectile->Attach())
		FatalError(L"Failed to place detour on target function(ArrowFixes::DebugTraceProjectile), this error is fatal.");

	detArrowSpawn = eastl::make_unique<ArrowSpawnFunc>(g_Offsets->DebugSpawnProjectile, mArrowSpawn);
	if (!detArrowSpawn->Attach())
		FatalError(L"Failed to place detour on target function(ArrowFixes::DebugSpawnProjectile), this error is fatal.");

	if (Render::HasContext())
		segmentDrawer = eastl::make_unique<Render::LineDrawer>(Render::GetContext());

	Detours::RegisterGameShutdownEvent([] {
		std::lock_guard<std::mutex> lock(segmentLock);
		segmentDrawer.reset();
	});
#endif

	return true;
}