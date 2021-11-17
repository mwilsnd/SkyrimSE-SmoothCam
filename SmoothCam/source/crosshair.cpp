#include "crosshair.h"
#include "crosshair/skyrim.h"
#include "crosshair/dot.h"
#ifdef DEBUG
#include "arrow_fixes.h"
#endif

extern Offsets* g_Offsets;

static Crosshair::Manager::CurrentCrosshairData g_crosshairData;

Crosshair::Manager::Manager() noexcept {
	ReadInitialCrosshairInfo();
	Messaging::SmoothCamInterface::GetInstance()->SetCrosshairManager(this);

	if (!Render::HasContext()) return;
	auto& ctx = Render::GetContext();

	// Per-object data, changing each draw call (model)
	Render::CBufferCreateInfo perObj;
	perObj.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	perObj.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	perObj.size = sizeof(decltype(renderables.cbufPerObjectStaging));
	perObj.initialData = &renderables.cbufPerObjectStaging;
	renderables.cbufPerObject = eastl::make_shared<Render::CBuffer>(perObj, ctx);

	// Per-frame data, shared among many objects (view, projection)
	Render::CBufferCreateInfo perFrane;
	perFrane.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	perFrane.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	perFrane.size = sizeof(decltype(renderables.cbufPerFrameStaging));
	perFrane.initialData = &renderables.cbufPerFrameStaging;
	renderables.cbufPerFrame = eastl::make_shared<Render::CBuffer>(perFrane, ctx);

	// Create our line drawer for the crosshair tail
	renderables.tailDrawer = eastl::make_unique<Render::LineDrawer>(ctx);
}

Crosshair::Manager::~Manager() {
	renderables.release();
}

glm::vec3 Crosshair::Manager::GetCrosshairTargetNormal(const glm::vec2& aimRotation, float pitchMod) const noexcept {
	return mmath::GetViewVector(
		glm::vec3(0.0f, 1.0f, 0.0f),
		aimRotation.x - pitchMod,
		aimRotation.y
	);
}

void Crosshair::Manager::TickProjectilePath(glm::vec3& position, glm::vec3& vel, const glm::vec3& gravity,
	float gravityScale, float dt) noexcept
{
	// From the iron arrow nif file
	// Until I can figure out a way to read these values at runtime, assume they don't change
	constexpr auto linDamp = 0.099609f;
	constexpr auto gravityFactor = 1.0f;

	const auto magic = glm::vec3{ 0.0f, 0.0f, 59.0f }; // just roll with it, i don't know either
	const glm::vec3 gravityVector = gravity * gravityFactor * gravityScale * magic;
	const auto step = gravityVector -(vel * linDamp);

	vel += step * dt;
	position += vel * dt;
}

glm::vec3 Crosshair::Manager::ComputeProjectileVelocityVector(const RE::Actor* player,
	const RE::TESAmmo* ammo, const glm::vec2& aimRotation) noexcept
{
	// Read the tilt angle
	float tilt = 0.0f;
	if (GameState::IsUsingBow(player) || GameState::IsUsingCrossbow(player)) {
		static auto arrowTilt = RE::INISettingCollection::GetSingleton()->GetSetting("f3PArrowTiltUpAngle:Combat");
		tilt = glm::radians(arrowTilt->GetFloat());
	}

	// Get the rotation for the arrow
	auto arrowRot = aimRotation;
	// Offset with tilt angle and invert
	arrowRot.x -= tilt;
	arrowRot.x *= -1;

	// Read required metrics for the shot

	// Replicates the function at 42537
	const auto s2 = [](const RE::TESObjectWEAP* wep) {
		if (!wep) return 1.0f;
		
		auto speed = wep->weaponData.speed;
		const auto DAT_141de0da8 = *REL::Relocation<float*>(g_Offsets->DAT_141de0da8);
		const auto DAT_141de0dc0 = *REL::Relocation<float*>(g_Offsets->DAT_141de0dc0);
		const auto DAT_142f01438 = *REL::Relocation<float*>(g_Offsets->DAT_142f01438);

		if (DAT_141de0da8 <= speed)
			speed = DAT_141de0da8;

		if (speed <= DAT_141de0dc0)
			speed = DAT_141de0dc0;

		return (DAT_141de0dc0 - speed) * DAT_142f01438 + 1.0f;
	}(GameState::GetEquippedWeapon(player));

	// Replicates the function at 42536
	const auto power = [](float unk188_ArrowDrawTimer) {
		// @Note: The parameter is projectile->unk188
		const float fVar1 = unk188_ArrowDrawTimer; // FUN_14074dc80(arrowProjectile);

		const auto DAT_141de0df0 = *REL::Relocation<float*>(g_Offsets->DAT_141de0df0);
		const auto DAT_141de0e08 = *REL::Relocation<float*>(g_Offsets->DAT_141de0e08);

		return ((fVar1 - DAT_141de0df0) / (1.0f - DAT_141de0df0)) * (1.0f - DAT_141de0e08) + DAT_141de0e08;
	}(GameState::GetCurrentBowDrawTimer(RE::PlayerCharacter::GetSingleton()));

	const auto speed = ammo->data.projectile->data.speed;
	// arrowProjectile->arrowUnk18C
	// Appears to always be one
	constexpr auto arrowUnk18C = 1.0f;

	// Now calculate the initial velocity vector for the arrow if we were to fire it right now
	const auto arrowFireSpeed = speed * power;
	const auto velScalar = s2 * arrowFireSpeed * arrowUnk18C;
	const auto pitchCos = glm::cos(arrowRot.x);
	const auto pitchSin = glm::sin(arrowRot.x);
	const auto yawCos = glm::cos(arrowRot.y);
	const auto yawSin = glm::sin(arrowRot.y);
	const auto pitchCosVel = pitchCos * velScalar;
	const auto pitchSinVel = pitchSin * velScalar;
	return {
		pitchCosVel * yawSin,
		pitchCosVel * yawCos,
		pitchSinVel
	};
}

bool Crosshair::Manager::ProjectilePredictionCurve(const RE::Actor* player, const glm::vec2& aimRotation,
	const glm::vec3& startPos, glm::vec3& hitPos, bool& hitCharacter) noexcept
{
	// Get the ammo we are using
	const auto ammo = player->GetCurrentAmmo();
	if (!ammo) return false;

	const auto config = Config::GetCurrentConfig();
	const auto gravityScale = ammo->data.projectile->data.gravity;
	auto gravity = glm::vec4{ 0.0f, 0.0f, -9.8f, 0.0f };
	if (player->parentCell && player->parentCell->GetbhkWorld() && player->parentCell->GetbhkWorld()->GetWorld())
		gravity = *reinterpret_cast<glm::vec4*>(&player->parentCell->GetbhkWorld()->GetWorld()->gravity);

	// Compute impulse
	glm::vec3 velocityVector = ComputeProjectileVelocityVector(player, ammo, aimRotation);

	// Max number of segments we want to simulate
	constexpr auto segCount = 128;
	constexpr float timeStep = 1.0f / 20.0f; // The slower the timestep, the more we decimate the arc
	glm::vec3 lastPos = startPos;
	glm::vec3 curPos = startPos;

	uint8_t entries = 0;
	eastl::array<eastl::tuple<glm::vec3, glm::vec3>, segCount> points;

	bool hit = false;
	for (auto i = 0; i < segCount; i++) {
		TickProjectilePath(curPos, velocityVector, gravity, gravityScale, timeStep);
		const auto origin = glm::vec4(lastPos.x, lastPos.y, lastPos.z, 0.0f);
		const auto endPoint = glm::vec4(curPos.x, curPos.y, curPos.z, 0.0f);
		const auto result = Raycast::hkpCastRay(origin, origin + (endPoint - origin));

		if (result.hit) {
			hitPos = static_cast<glm::vec3>(result.hitPos);
			points[i] = { lastPos, hitPos };
			entries++;
			hit = true;
			hitCharacter = result.hitCharacter != nullptr;
			break;
		}

		points[i] = { lastPos, curPos };
		entries++;
		lastPos = curPos;
		// Went past max distance, bail
		if (glm::distance(startPos, curPos) > config->maxArrowPredictionRange)
			break;
	}

	// Now populate our final line list for drawing the arc
	// Check user supplied alpha - if 0, why bother?
	// Only submit lines if we have a render context, otherwise the line buffer will never be flushed
	if (hit && config->drawArrowArc && Render::HasContext() && config->arrowArcColor.a > 0.0f) {
		// Convert user color from 0-255 to 0-1
		const auto col = Config::Color(
			mmath::Remap(config->arrowArcColor.r, 0.0f, 255.0f, 0.0f, 1.0f),
			mmath::Remap(config->arrowArcColor.g, 0.0f, 255.0f, 0.0f, 1.0f),
			mmath::Remap(config->arrowArcColor.b, 0.0f, 255.0f, 0.0f, 1.0f),
			mmath::Remap(config->arrowArcColor.a, 0.0f, 255.0f, 0.0f, 1.0f)
		);

		const auto max = static_cast<float>(entries);
		for (auto i = 0; i < entries; i++) {
			const auto& [l1, l2] = points[i];
			// Have the alpha fade in over distance
			const auto cur = static_cast<float>(i);
			const auto dt = max - cur;
			const auto alpha = dt <= 0.0f ? 0.0f : dt / max;
			const auto dt2 = max - (cur + 1);
			const auto alpha2 = dt2 <= 0.0f ? 0.0f : dt2 / max;
			renderables.arrowTailSegments.emplace_back(
				Render::Point(
					Render::ToRenderScale(l1),
					{
						col.r, col.g, col.b,
						(1.0f - glm::clamp(alpha, 0.0f, 1.0f)) * col.a
					}
				),
				Render::Point(
					Render::ToRenderScale(l2),
					{
						col.r, col.g, col.b,
						(1.0f - glm::clamp(alpha2, 0.0f, 1.0f)) * col.a
					}
				)
			);
		}
	}

	return hit;
}

RE::NiAVObject* Crosshair::Manager::FindArrowNode(const RE::Actor* player) const noexcept {
	if (GameState::IsUsingCrossbow(player)) {
		auto rmag = player->loadedData->data3D->GetObjectByName(Strings.rmag);
		if (rmag) {
			// So on horseback, Arrow:0 doesn't appear for whatever reason
			auto arrow = rmag->GetObjectByName(Strings.arrowName);
			if (arrow)
				return arrow;

			// Se we need to pick the next best location, which seems to be rmag
			return rmag;
		}
	}

	auto handNode = skyrim_cast<RE::NiNode*>(player->loadedData->data3D->GetObjectByName(Strings.weapon));
	if (handNode && handNode->children.size() > 0) {
		auto arrow = handNode->GetObjectByName(Strings.arrowName);
		if (arrow) return arrow;
	}

	handNode = skyrim_cast<RE::NiNode*>(player->loadedData->data3D->GetObjectByName(Strings.rmag));
	if (handNode && handNode->children.size() > 0) {
		auto arrow = handNode->GetObjectByName(Strings.arrowName);
		if (arrow) return arrow;
	}

	handNode = skyrim_cast<RE::NiNode*>(player->loadedData->data3D->GetObjectByName(Strings.lmag));
	if (handNode && handNode->children.size() > 0) {
		auto arrow = handNode->GetObjectByName(Strings.arrowName);
		if (arrow) return arrow;
	}

	return nullptr;
}

glm::vec3 Crosshair::Manager::TranslateFirePostion(const RE::Actor* player, const glm::vec2& cameraRotation,
	const glm::vec3& firePos) const noexcept
{
	if (GameState::IsInHorseCamera(RE::PlayerCamera::GetSingleton()))
		return firePos;

	const auto& playerRef = player->loadedData->data3D->world;
	const auto playerPos = glm::vec3{ playerRef.translate.x, playerRef.translate.y, playerRef.translate.z };
	const auto localSpace = firePos - playerPos;
	const auto yawDelta = player->data.angle.z - cameraRotation.y;

	auto inv = glm::rotate(glm::identity<glm::mat4>(), yawDelta, { 0.0f, 0.0f, 1.0f });
	auto local = inv * glm::vec4{ localSpace.x, localSpace.y, 0.0f, 1.0f };
	local.z += localSpace.z;

	return playerPos + static_cast<glm::vec3>(local);
}

void Crosshair::Manager::UpdateCrosshairPosition(const RE::Actor* player, const glm::vec2& aimRotation,
	const glm::vec2& cameraRotation, mmath::NiMatrix44& worldToScaleform) noexcept
{
	if (Messaging::SmoothCamInterface::GetInstance()->IsCrosshairTaken()) return;
	if (!player->loadedData || !player->loadedData->data3D) return;

	const auto config = Config::GetCurrentConfig();
	auto maxRayLength = 8000.0f;
	glm::vec3 hitPos = { 0, 0, 0 };
	float rayLength = 0.0f;
	bool hit = false;
	bool hitCharacter = false;

	if (GameState::IsBowDrawn(player)) {
		const auto arrow = FindArrowNode(player);
		if (arrow) {
			const auto pos = TranslateFirePostion(player, cameraRotation, glm::vec3{
				arrow->world.translate.x,
				arrow->world.translate.y,
				arrow->world.translate.z
			});

			// Now select the method to use
			if (config->useArrowPrediction) {
				maxRayLength = config->maxArrowPredictionRange;
				if (ProjectilePredictionCurve(player, aimRotation, pos, hitPos, hitCharacter)) {
					hit = true;
					rayLength = glm::length(hitPos - pos);
				}
			} else {
				// Classic method
				float fac = 0.0f;
				if (GameState::IsUsingBow(player) || GameState::IsUsingCrossbow(player)) {
					static auto arrowTilt = RE::INISettingCollection::GetSingleton()->GetSetting("f3PArrowTiltUpAngle:Combat");
					fac = glm::radians(arrowTilt->GetFloat()) * 0.5f;
				}

				const auto n = GetCrosshairTargetNormal(aimRotation, fac);
				const auto origin = glm::vec4(pos.x, pos.y, pos.z, 0.0f);
				const auto ray = glm::vec4(n.x, n.y, n.z, 0.0f) * maxRayLength;
				const auto result = Raycast::hkpCastRay(origin, origin + ray);
				hit = result.hit;
				hitPos = result.hitPos;
				rayLength = result.rayLength;
				hitCharacter = result.hitCharacter != nullptr;
			}
		}
	} else if (GameState::IsMagicDrawn(player)) {
		RE::NiPoint3 niOrigin = { 0.01f, 0.01f, 0.01f };
		glm::vec3 normal = { 0.0f, 1.00f, 0.0f };

		const auto handNode = skyrim_cast<RE::NiNode*>(player->loadedData->data3D->GetObjectByName(Strings.magic));
		if (handNode)
			niOrigin = { handNode->world.translate.x, handNode->world.translate.y, handNode->world.translate.z };

		normal = mmath::GetViewVector(
			glm::vec3(0.0f, 1.0f, 0.0f),
			aimRotation.x,
			cameraRotation.y
		);

		const auto pos = TranslateFirePostion(player, cameraRotation, {
			niOrigin.x, niOrigin.y, niOrigin.z
		});

		// Cast the aim ray
		const auto origin = glm::vec4(pos, 0.0f);
		const auto ray = glm::vec4(normal.x, normal.y, normal.z, 0.0f) * maxRayLength;
		const auto result = Raycast::hkpCastRay(origin, origin + ray);
		hit = result.hit;
		hitPos = result.hitPos;
		rayLength = result.rayLength;
		hitCharacter = result.hitCharacter != nullptr;
	}

	// Now set the crosshair
	if (hit) {
		// Flag our crosshair for drawing, if we have one
		if (config->useWorldCrosshair && renderables.curCrosshair) {
			SetCrosshairEnabled(false);

			// Give us a bit of breathing room if using the bow - we don't want to draw the crosshair
			// on the player's back for the first few frames.
			if (GameState::IsBowDrawn(player) && rayLength <= 15.0f) return;

			renderables.drawCrosshair = true;
			renderables.hitCharacter = hitCharacter;
			renderables.curCrosshair->SetPosition(hitPos);
			// We want the crosshair to face the player
			renderables.curCrosshair->SetRotation({
				mmath::half_pi - cameraRotation.x,
				cameraRotation.y * -1.0f,
				0.0f
			});

			return;
		}
		
		const auto pt = RE::NiPoint3(hitPos.x, hitPos.y, hitPos.z);
		const auto rangeScalar = glm::clamp((maxRayLength - rayLength) / maxRayLength, 0.0f, 1.0f);
		const auto sz = mmath::Remap(rangeScalar, 0.0f, 1.0f, config->crosshairMinDistSize, config->crosshairMaxDistSize);
		glm::vec2 crosshairSize = { sz, sz };

		// Use the HUD crosshair
		auto port = RE::NiRect<float>();
		auto menu = RE::UI::GetSingleton()->GetMenu(RE::InterfaceStrings::GetSingleton()->hudMenu);
		if (menu && menu->uiMovie) {
			const auto rect = menu->uiMovie->GetVisibleFrameRect();
			port = RE::NiRect<float>(rect.left, rect.right, rect.top, rect.bottom);
		}

		glm::vec3 screen = {};
		RE::NiCamera::WorldPtToScreenPt3(
			worldToScaleform.data,
			port, pt,
			screen.x, screen.y, screen.z, 9.99999975e-06f
		);

		if (hitCharacter)
			crosshairSize += config->crosshairNPCHitGrowSize * rangeScalar;

		glm::vec2 crosshairPos = {
			screen.x,
			screen.y
		};

		SetCrosshairEnabled(true);
		SetCrosshairPosition(crosshairPos);
		SetCrosshairSize(crosshairSize);
	} else {
		SetCrosshairEnabled(true);
		CenterCrosshair();
		SetDefaultSize();
	}
}

bool Crosshair::Manager::IsCrosshairDataValid() const noexcept {
	return baseCrosshairData.captured;
}

void Crosshair::Manager::ReadInitialCrosshairInfo() noexcept {
	auto menu = RE::UI::GetSingleton()->GetMenu(RE::InterfaceStrings::GetSingleton()->hudMenu);
	if (!menu || !menu->uiMovie) return;

	RE::GFxValue va;
	menu->uiMovie->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._x");
	baseCrosshairData.xOff = va.GetNumber();

	menu->uiMovie->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._y");
	baseCrosshairData.yOff = va.GetNumber();

	menu->uiMovie->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._width");
	baseCrosshairData.xScale = va.GetNumber();

	menu->uiMovie->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._height");
	baseCrosshairData.yScale = va.GetNumber();

	const auto rect = menu->uiMovie->GetVisibleFrameRect();
	baseCrosshairData.xCenter = mmath::Remap(0.5f, 0.0f, 1.0f, rect.left, rect.right);
	baseCrosshairData.yCenter = mmath::Remap(0.5f, 0.0f, 1.0f, rect.top, rect.bottom);

	menu->uiMovie->GetVariable(&va, "_root.HUDMovieBaseInstance.StealthMeterInstance._x");
	baseCrosshairData.stealthXOff = va.GetNumber();

	menu->uiMovie->GetVariable(&va, "_root.HUDMovieBaseInstance.StealthMeterInstance._y");
	baseCrosshairData.stealthYOff = va.GetNumber();

	baseCrosshairData.captured = true;
	currentCrosshairData.enabled = true;

	currentCrosshairData.position = currentCrosshairData.stealthMeterPosition = {
		baseCrosshairData.xCenter,
		baseCrosshairData.yCenter
	};
	currentCrosshairData.scale = {
		baseCrosshairData.xScale,
		baseCrosshairData.yScale
	};

	g_crosshairData = currentCrosshairData;
	g_crosshairData.ofs = { baseCrosshairData.xOff, baseCrosshairData.yOff };
	g_crosshairData.stealthMeterOfs = { baseCrosshairData.stealthXOff, baseCrosshairData.stealthYOff };
}

void Crosshair::Manager::SetCrosshairPosition(const glm::dvec2& pos) noexcept {
	if (!IsCrosshairDataValid()) return;
	if (currentCrosshairData.position == pos) return;

	auto menu = RE::UI::GetSingleton()->GetMenu(RE::InterfaceStrings::GetSingleton()->hudMenu);
	if (menu && menu->uiMovie) {
		const auto rect = menu->uiMovie->GetVisibleFrameRect();
		const auto half_x = pos.x -
			((static_cast<double>(rect.right) + static_cast<double>(rect.left)) * 0.5);
		const auto half_y = pos.y -
			((static_cast<double>(rect.bottom) + static_cast<double>(rect.top)) * 0.5);

		const auto x = half_x + baseCrosshairData.xOff;
		const auto y = half_y + baseCrosshairData.yOff;
		RE::GFxValue::DisplayInfo loc;
		loc.SetPosition(x, y);
		
		RE::GFxValue var;
		menu->uiMovie->GetVariable(&var, "_root.HUDMovieBaseInstance.Crosshair");
		if (var.IsDisplayObject())
			var.SetDisplayInfo(loc);
		
		currentCrosshairData.position = pos;
		g_crosshairData.position = pos;
		g_crosshairData.ofs = { x, y };
	}
}

void Crosshair::Manager::SetStealthMeterPosition(const glm::vec2& pos) noexcept {
	if (!IsCrosshairDataValid()) return;
	auto menu = RE::UI::GetSingleton()->GetMenu(RE::InterfaceStrings::GetSingleton()->hudMenu);
	if (menu && menu->uiMovie) {
		RE::GFxValue::DisplayInfo loc;
		loc.SetPosition(
			baseCrosshairData.stealthXOff + static_cast<double>(pos.x),
			baseCrosshairData.stealthYOff + static_cast<double>(pos.y)
		);

		RE::GFxValue var;
		menu->uiMovie->GetVariable(&var, "_root.HUDMovieBaseInstance.StealthMeterInstance");
		if (var.IsDisplayObject())
			var.SetDisplayInfo(loc);

		currentCrosshairData.stealthMeterPosition = pos;
		g_crosshairData.stealthMeterPosition = pos;
		currentCrosshairData.stealthMeterMutated = true;
	}
}

void Crosshair::Manager::CenterCrosshair() noexcept {
	if (!IsCrosshairDataValid()) return;
	SetCrosshairPosition({
		baseCrosshairData.xCenter,
		baseCrosshairData.yCenter
	});
}

void Crosshair::Manager::CenterStealthMeter() noexcept {
	if (!IsCrosshairDataValid()) return;
	SetStealthMeterPosition({0.0, 0.0});
	currentCrosshairData.stealthMeterMutated = false;
}

void Crosshair::Manager::SetCrosshairSize(const glm::dvec2& size) noexcept {
	if (!IsCrosshairDataValid()) return;
	if (currentCrosshairData.scale == size) return;
	if (!Config::GetCurrentConfig()->enableCrosshairSizeManip) return;

	auto menu = RE::UI::GetSingleton()->GetMenu(RE::InterfaceStrings::GetSingleton()->hudMenu);
	if (menu && menu->uiMovie) {
		RE::GFxValue::DisplayInfo loc;
		loc.SetScale(size.x, size.y);

		RE::GFxValue var;
		menu->uiMovie->GetVariable(&var, "_root.HUDMovieBaseInstance.Crosshair");
		if (var.IsDisplayObject())
			var.SetDisplayInfo(loc);

		currentCrosshairData.scale = size;
		g_crosshairData.scale = size;
	}
}

void Crosshair::Manager::SetDefaultSize() noexcept {
	if (!IsCrosshairDataValid()) return;
	SetCrosshairSize({
		baseCrosshairData.xScale,
		baseCrosshairData.yScale
	});
}

void Crosshair::Manager::SetCrosshairEnabled(bool enabled) noexcept {
	if (!IsCrosshairDataValid()) return;
	if (currentCrosshairData.enabled == enabled && !currentCrosshairData.invalidated) return;

	auto menu = RE::UI::GetSingleton()->GetMenu(RE::InterfaceStrings::GetSingleton()->hudMenu);
	if (menu && menu->uiMovie && menu->fxDelegate) {
		RE::FxResponseArgsEx<1> args;
		args[0].SetBoolean(enabled);
		menu->fxDelegate->Invoke(menu->uiMovie.get(), "SetCrosshairEnabled", args);

		currentCrosshairData.enabled = enabled;
		currentCrosshairData.invalidated = false;
		g_crosshairData.enabled = enabled;
	}
}

void Crosshair::Manager::Set3DCrosshairType(Config::CrosshairType type) noexcept {
	if (!Render::HasContext()) return;
	if (renderables.crosshairType == type) return;

	// Crosshair either changed or this is the first time running
	switch (type) {
		case Config::CrosshairType::Skyrim:
			renderables.curCrosshair = eastl::make_unique<Crosshair::Skyrim>();
			break;
		case Config::CrosshairType::Dot:
			renderables.curCrosshair = eastl::make_unique<Crosshair::Dot>();
			break;
		default:
			renderables.curCrosshair = eastl::make_unique<Crosshair::Skyrim>();
			break;
	}

	renderables.curCrosshair->Create3D(Render::GetContext(), renderables.cbufPerObject);
	renderables.crosshairType = type;
}

void Crosshair::Manager::InvalidateEnablementCache() noexcept {
	currentCrosshairData.invalidated = true;
}

void Crosshair::Manager::Update(RE::Actor* player) noexcept {
	if (!IsCrosshairDataValid()) return;

	if (!Messaging::SmoothCamInterface::GetInstance()->IsCrosshairTaken())
		if (Messaging::SmoothCamInterface::GetInstance()->CrosshairDirty()) {
			ResetCrosshair();
			Messaging::SmoothCamInterface::GetInstance()->ClearCrosshairDirtyFlag();
		}

	if (Messaging::SmoothCamInterface::GetInstance()->IsStealthMeterTaken()) return;
	if (Messaging::SmoothCamInterface::GetInstance()->StealthMeterDirty()) {
		ResetStealthMeter();
		Messaging::SmoothCamInterface::GetInstance()->ClearStealthMeterDirtyFlag();
	}

	// Stealth meter offset
	const auto config = Config::GetCurrentConfig();

	if (!config->offsetStealthMeter) {
		if (currentCrosshairData.stealthMeterMutated)
			CenterStealthMeter();
		return;
	}

	if (!GameState::IsSneaking(player)) {
		if (currentCrosshairData.stealthMeterMutated) {
			CenterStealthMeter();
		}
		return;
	}

	const auto usingBow = GameState::IsBowDrawn(player);
	const auto usingMagic = GameState::IsMagicDrawn(player);
	if ((config->useWorldCrosshair && (usingBow || usingMagic)) || config->alwaysOffsetStealthMeter) {
		SetStealthMeterPosition({
			config->stealthMeterXOffset,
			config->stealthMeterYOffset
		});

	} else if (currentCrosshairData.stealthMeterMutated) {
		CenterStealthMeter();
	}
}

void Crosshair::Manager::Render(Render::D3DContext& ctx, const glm::vec3& cameraPosition, const glm::vec2& cameraRotation,
	const RE::NiFrustum& frustum) noexcept
{
	if (!IsCrosshairDataValid()) {
		// Try and capture now
		ReadInitialCrosshairInfo();
	}

	const auto config = Config::GetCurrentConfig();

	// Only update GPU if we are going to draw
#ifdef DEBUG
	if (1) {
#else
	if (renderables.arrowTailSegments.size() > 0 || (renderables.drawCrosshair && renderables.curCrosshair)) {
#endif
		// Update view, projection and common per frame data
		const auto matProj = Render::GetProjectionMatrix(frustum);
		const auto matView = Render::BuildViewMatrix(cameraPosition, cameraRotation);
		renderables.cbufPerFrameStaging.matProjView = matProj * matView;

		// Color tinting for crosshair alert mode (ex: conjuration)
		renderables.cbufPerFrameStaging.tint = g_crosshairData.alertMode ?
			glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f } :
			glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

		renderables.cbufPerFrameStaging.curTime = static_cast<float>(GameTime::CurTime());
		renderables.cbufPerFrame->Update(
			&renderables.cbufPerFrameStaging, 0,
			sizeof(decltype(renderables.cbufPerFrameStaging)), ctx
		);

		// Bind at register b1
		renderables.cbufPerFrame->Bind(Render::PipelineStage::Vertex, 1, ctx);
		renderables.cbufPerFrame->Bind(Render::PipelineStage::Fragment, 1, ctx);

		// Setup depth and blending
		Render::SetDepthState(ctx, true, true, D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL);
		Render::SetBlendState(
			ctx, true,
			D3D11_BLEND_OP::D3D11_BLEND_OP_ADD, D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
			D3D11_BLEND::D3D11_BLEND_SRC_ALPHA, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND::D3D11_BLEND_ONE, D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA
		);
	}

	if (renderables.arrowTailSegments.size() > 0) {
		renderables.tailDrawer->Submit(renderables.arrowTailSegments);
		renderables.arrowTailSegments.clear();
	}

#ifdef DEBUG
	// Draw arrow debug overlay here, render state is already configured
	ArrowFixes::Draw(ctx);
#endif

	if (renderables.drawCrosshair && renderables.curCrosshair) {
		// Invert the scaling factor from our perspective projection to give the crosshair an
		// apparent constant size at any distance
		const auto d = glm::distance(cameraPosition, renderables.curCrosshair->GetPosition());
		const auto pScale = (2.0f * glm::tan(Render::GetFOV() * 0.5f)) * d;

		float s = 27.5f;
		if (config->enableCrosshairSizeManip) {
			auto rangeScalar = glm::clamp((8000.0f - d) / 8000.0f, 0.0f, 1.0f);
			auto sz = mmath::Remap(rangeScalar, 0.0f, 1.0f, config->crosshairMinDistSize, config->crosshairMaxDistSize);
			s = sz;

			if (renderables.hitCharacter) {
				s += config->crosshairNPCHitGrowSize;
			}
		}

		renderables.curCrosshair->SetScale({
			(pScale * 0.001f) * s * 0.0055f,
			(pScale * 0.001f) * s * 0.0055f,
			1.0f
		});

		renderables.curCrosshair->Render(ctx, config->worldCrosshairDepthTest);
		renderables.drawCrosshair = false;
		renderables.hitCharacter = false;
	}
}

void Crosshair::Manager::Reset(bool hard) noexcept {
	if (!IsCrosshairDataValid()) return;
	if (hard) {
		Config::GetCurrentConfig()->hideCrosshairMeleeCombat = false;
		Config::GetCurrentConfig()->hideNonCombatCrosshair = false;
		Config::GetCurrentConfig()->useWorldCrosshair = false;
		Config::GetCurrentConfig()->use3DBowAimCrosshair = false;
		Config::GetCurrentConfig()->use3DMagicCrosshair = false;
	}

	ResetCrosshair();
	ResetStealthMeter(hard);
}

void Crosshair::Manager::ResetCrosshair() noexcept {
	if (!IsCrosshairDataValid()) return;
	InvalidateEnablementCache();
	SetCrosshairEnabled(true);
	SetDefaultSize();
	CenterCrosshair();
}

void Crosshair::Manager::ResetStealthMeter(bool hard) noexcept {
	if (!IsCrosshairDataValid()) return;
	if (currentCrosshairData.stealthMeterMutated || hard)
		CenterStealthMeter();
	currentCrosshairData.stealthMeterMutated = false;
}

Crosshair::Manager::CurrentCrosshairData& Crosshair::Manager::GetCurrentCrosshairData() noexcept {
	return currentCrosshairData;
}

void Crosshair::Manager::SetAlertMode(bool alert) noexcept {
	g_crosshairData.alertMode = alert;
}

void Crosshair::Manager::ValidateCrosshair() noexcept {
	auto menu = RE::UI::GetSingleton()->GetMenu(RE::InterfaceStrings::GetSingleton()->hudMenu);
	if (menu && menu->uiMovie && menu->fxDelegate) {
		RE::FxResponseArgsEx<1> arg;
		arg[0].SetBoolean(g_crosshairData.enabled);
		menu->fxDelegate->Invoke(menu->uiMovie.get(), "SetCrosshairEnabled", arg);

		RE::GFxValue::DisplayInfo loc;
		loc.SetPosition(
			g_crosshairData.ofs.x,
			g_crosshairData.ofs.y
		);
		loc.SetScale(
			g_crosshairData.scale.x,
			g_crosshairData.scale.y
		);

		RE::GFxValue var;
		menu->uiMovie->GetVariable(&var, "_root.HUDMovieBaseInstance.Crosshair");
		if (var.IsDisplayObject())
			var.SetDisplayInfo(loc);
	}
}