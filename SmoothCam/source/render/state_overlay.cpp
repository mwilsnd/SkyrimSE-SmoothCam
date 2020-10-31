#ifdef WITH_D2D
#include "render/state_overlay.h"
#include "render/d2d.h"
#include "render/dwrite.h"
#include "camera.h"

extern std::unique_ptr<Render::D2D> g_D2D;

static const wchar_t* ActionStateEnumNames[] = {
	L"Standing",
	L"Walking",
	L"Running",
	L"Sprinting",
	L"Sneaking",
	L"Swimming",
	L"SittingTransition",
	L"Sitting",
	L"FirstPersonSitting",
	L"Sleeping",
	L"Aiming",
	L"Mounting",
	L"DisMounting",
	L"FirstPersonHorseback",
	L"FirstPersonDragon",
	L"VampireLord",
	L"Werewolf",
	L"Unknown",
};
constexpr auto actionNameLen = sizeof(ActionStateEnumNames) / sizeof(ActionStateEnumNames[0]);
static_assert(actionNameLen == static_cast<size_t>(Camera::CameraActionState::MAX_STATE));

static const wchar_t* CameraStateEnumNames[] = {
	L"FirstPerson",
	L"ThirdPerson",
	L"ThirdPersonCombat",
	L"KillMove",
	L"Tweening",
	L"Transitioning",
	L"UsingObject",
	L"Vanity",
	L"Free",
	L"IronSights",
	L"Furniture",
	L"Horseback",
	L"Bleedout",
	L"Dragon",
	L"Unknown"
};
constexpr auto stateNameLen = sizeof(CameraStateEnumNames) / sizeof(CameraStateEnumNames[0]);
static_assert(stateNameLen == static_cast<size_t>(GameState::CameraState::MAX_STATE));

static const wchar_t* OffsetGroupNames[]{
	L"Standing",
	L"Walking",
	L"Running",
	L"Sprinting",
	L"Sneaking",
	L"Swimming",
	L"BowAim",
	L"Sitting",
	L"Horseback",
	L"Dragon",
	L"VampireLord",
	L"Werewolf",
	L"Unknown",
};
constexpr auto ofsNameLen = sizeof(OffsetGroupNames) / sizeof(OffsetGroupNames[0]);
static_assert(ofsNameLen == static_cast<size_t>(Config::OffsetGroupID::MAX_OFS));

Render::StateOverlay::StateOverlay(uint32_t width, uint32_t height, Camera::SmoothCamera* camera,
	D3DContext& ctx) : width(width), height(height), camera(camera), GradBox(ctx, width, height)
{}

Render::StateOverlay::~StateOverlay() {}

void Render::StateOverlay::SetPosition(uint32_t x, uint32_t y) noexcept {
	xPos = x;
	yPos = y;
	SetBackgroundPosition({ x, y });
}

void Render::StateOverlay::SetSize(uint32_t w, uint32_t h) noexcept {
	width = w;
	height = h;
	SetBackgroundSize({ w, h });
}

void Render::StateOverlay::Draw(const Config::OffsetGroup* curGroup, D3DContext& ctx) noexcept {
	DrawBackground(ctx);

	constexpr auto lineHeight = 14.0f;
	glm::vec2 curPos = { 5.0f + xPos, 0.0f + yPos };
	auto ply = *g_thePlayer;
	
	g_D2D->GetDWrite()->Write(
		L"Player State Bits",
		ctx.windowSize.x, ctx.windowSize.y,
		curPos,
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);
	curPos.y += lineHeight;

	DrawBitset32(L"Movement Bits: ", GameState::GetPlayerMovementBits(ply), curPos, ctx);
	curPos.y += lineHeight;

	DrawBitset32(L"Action Bits: ", GameState::GetPlayerActionBits(ply), curPos, ctx);
	curPos.y += lineHeight;

	std::wstring cameraState = L"Camera State: ";
	cameraState.append(CameraStateEnumNames[static_cast<uint8_t>(camera->GetCurrentCameraState())]);
	g_D2D->GetDWrite()->Write(
		cameraState.c_str(),
		ctx.windowSize.x, ctx.windowSize.y,
		curPos,
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);
	curPos.y += lineHeight;

	std::wstring actionState = L"Action State: ";
	actionState.append(ActionStateEnumNames[static_cast<uint8_t>(camera->GetCurrentCameraActionState())]);
	g_D2D->GetDWrite()->Write(
		actionState.c_str(),
		ctx.windowSize.x, ctx.windowSize.y,
		curPos,
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);
	curPos.y += lineHeight;

	DrawBool(L"Weapon Drawn",			GameState::IsWeaponDrawn(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Using Left Magic Item",	GameState::IsUsingMagicItem(ply, true), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Using Right Magic Item",	GameState::IsUsingMagicItem(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Left Combat Magic",		GameState::IsCombatMagic(ply->leftHandSpell), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Right Combat Magic",		GameState::IsCombatMagic(ply->rightHandSpell), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Melee Drawn",			GameState::IsMeleeWeaponDrawn(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Magic Drawn",			GameState::IsMagicDrawn(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Ranged Drawn",			GameState::IsRangedWeaponDrawn(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Using Crossbow",			GameState::IsUsingCrossbow(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Using Bow",				GameState::IsUsingBow(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Aiming With Bow",		GameState::IsBowDrawn(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Sitting",				GameState::IsSitting(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Sleeping",				GameState::IsSleeping(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Swimming",				GameState::IsSwimming(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Mounting",				GameState::IsMountingHorse(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"Dismounting",			GameState::IsDisMountingHorse(ply), curPos, ctx);
	curPos.y += lineHeight;
	DrawBool(L"POV Slide Mode",			GameState::InPOVSlideMode(), curPos, ctx);
	curPos.y += lineHeight;

	curPos.x = xPos + 270;
	curPos.y = yPos + lineHeight * 4;

	const auto cam = CorrectedPlayerCamera::GetSingleton();
	const auto focus = camera->GetCurrentCameraTarget(cam);
	if (focus) {
		DrawBool(L"IsFirstPerson", GameState::IsFirstPerson(focus, cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsThirdPerson", GameState::IsThirdPerson(focus, cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsThirdPersonCombat", GameState::IsThirdPersonCombat(ply, cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInKillMove", GameState::IsInKillMove(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInTweenCamera", GameState::IsInTweenCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInCameraTransition", GameState::IsInCameraTransition(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInUsingObjectCamera", GameState::IsInUsingObjectCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInAutoVanityCamera", GameState::IsInAutoVanityCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInFreeCamera", GameState::IsInFreeCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInAimingCamera", GameState::IsInAimingCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInFurnitureCamera", GameState::IsInFurnitureCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInHorseCamera", GameState::IsInHorseCamera(focus, cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInBleedoutCamera", GameState::IsInBleedoutCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
		DrawBool(L"IsInDragonCamera", GameState::IsInDragonCamera(cam), curPos, ctx);
		curPos.y += lineHeight;
	}

	curPos.x = xPos + 160;
	curPos.y = yPos;

	if (!curGroup) {
		g_D2D->GetDWrite()->Write(
			L"No offset group!",
			ctx.windowSize.x, ctx.windowSize.y,
			curPos,
			{ 1.0f, 0.33f, 0.33f, 1.0f }
		);
	} else {
		std::wstring ofs = L"Offset Group: ";
		ofs.append(OffsetGroupNames[static_cast<uint8_t>(curGroup->id)]);
		g_D2D->GetDWrite()->Write(
			ofs,
			ctx.windowSize.x, ctx.windowSize.y,
			curPos,
			{ 1.0f, 1.0f, 1.0f, 1.0f }
		);
	}
}

void Render::StateOverlay::DrawBitset32(const std::wstring& name, const std::bitset<32>& bits,
	const glm::vec2& pos, D3DContext& ctx) noexcept
{
	std::wstring str;
	str.reserve(32);

	for (auto i = 0; i < 32; i++) {
		str.append(std::to_wstring(bits[i]));
	}

	g_D2D->GetDWrite()->Write(
		name,
		ctx.windowSize.x, ctx.windowSize.y,
		pos,
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);

	const auto sz = g_D2D->GetDWrite()->GetTextSize(name, ctx.windowSize.x, ctx.windowSize.y);
	g_D2D->GetDWrite()->Write(
		str,
		ctx.windowSize.x, ctx.windowSize.y,
		pos + glm::vec2{ sz.x, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	);
}

void Render::StateOverlay::DrawBool(const std::wstring& name, bool value, const glm::vec2& pos,
	D3DContext& ctx) noexcept
{
	std::wstring str;
	str.reserve(name.length() + 10);
	str.append(name);
	str.append(L" = ");
	str.append(value ? L"true" : L"false");

	g_D2D->GetDWrite()->Write(
		str,
		ctx.windowSize.x, ctx.windowSize.y,
		pos,
		value ?
			glm::vec4{ 0.33f, 1.0f, 0.33f, 1.0f } :
			glm::vec4{ 1.0f, 0.33f, 0.33f, 1.0f }
	);
}
#endif