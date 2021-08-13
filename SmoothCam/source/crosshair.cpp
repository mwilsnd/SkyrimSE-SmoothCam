#include "crosshair.h"
#include "crosshair/skyrim.h"
#include "crosshair/dot.h"

static Crosshair::Manager::CurrentCrosshairData g_crosshairData;

// 80230 : gotoAndStop
typedef uintptr_t(__fastcall* CrosshairInvoke)(GFxMovieView** param1, uint64_t* param2, const char* name, uint64_t param4);
static eastl::unique_ptr<TypedDetour<CrosshairInvoke>> detCrosshairInvoke;
uintptr_t __fastcall mCrosshairInvoke(GFxMovieView** param1, uint64_t* param2, const char* name, uint64_t param4) {
	const auto ret = detCrosshairInvoke->GetBase()(param1, param2, name, param4);
	if (!name) return ret;

	// @Note: This method is called right before "GFxInvoke" below
	if (strcmp(name, "Alert") == 0) {
		g_crosshairData.alertMode = true;
	} else if (strcmp(name, "Normal") == 0) {
		g_crosshairData.alertMode = false;
	}

	return ret;
}

//{ 0x00ECA860, 80233 },
typedef bool(__thiscall *GFxInvoke)(void* pThis, void* obj, GFxValue* result, const char* name, GFxValue* args, UInt32 numArgs, bool isDisplayObj);
static eastl::unique_ptr<TypedDetour<GFxInvoke>> detGFxInvoke;
bool __fastcall mGFxInvoke(void* pThis, void* obj, GFxValue* result, const char* name, GFxValue* args, UInt32 numArgs, bool isDisplayObj) {
	const auto ret = detGFxInvoke->GetBase()(pThis, obj, result, name, args, numArgs, isDisplayObj);

	if (!Messaging::SmoothCamAPIV1::GetInstance()->IsCrosshairTaken() && name && strcmp(name, "ValidateCrosshair") == 0) {
		// Getting spammed on by conjuration magic mode - (ノಠ益ಠ)ノ彡┻━┻
		// @Note: I really don't like these more invasive detours - finding a way around this should be a priority
		// We need this currently because ValidateCrosshair will desync our crosshair state and mess things up
		auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
		GFxValue instance;
		if (menu && menu->view && menu->view->GetVariable(&instance, "_root.HUDMovieBaseInstance.Crosshair")) {
			GFxValue result;
			GFxValue args[2];
			args[0].SetString("SetCrosshairEnabled");
			args[1].SetBool(g_crosshairData.enabled);
			menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 2);

			GFxValue member;
			if (instance.GetMember("_x", &member)) {
				member.SetNumber(g_crosshairData.ofs.x);
				instance.SetMember("_x", &member);
			}

			if (instance.GetMember("_y", &member)) {
				member.SetNumber(g_crosshairData.ofs.y);
				instance.SetMember("_y", &member);
			}

			if (instance.GetMember("_width", &member)) {
				member.SetNumber(g_crosshairData.scale.x);
				instance.SetMember("_width", &member);
			}

			if (instance.GetMember("_height", &member)) {
				member.SetNumber(g_crosshairData.scale.y);
				instance.SetMember("_height", &member);
			}

			menu->view->SetVariable("_root.HUDMovieBaseInstance.Crosshair", &instance, 2);
		}
	}

	return ret;
}

Crosshair::Manager::Manager() noexcept {
	detGFxInvoke = eastl::make_unique<TypedDetour<GFxInvoke>>(80233, mGFxInvoke);
	if (!detGFxInvoke->Attach()) {
		_ERROR("Failed to place detour on target function(80,233), this error is fatal.");
		FatalError(L"Failed to place detour on target function(80,233), this error is fatal.");
	}

	detCrosshairInvoke = eastl::make_unique<TypedDetour<CrosshairInvoke>>(80230, mCrosshairInvoke);
	if (!detCrosshairInvoke->Attach()) {
		_ERROR("Failed to place detour on target function(80,230), this error is fatal.");
		FatalError(L"Failed to place detour on target function(80,230), this error is fatal.");
	}

	ReadInitialCrosshairInfo();

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

	Messaging::SmoothCamAPIV1::GetInstance()->SetCrosshairManager(this);
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
	float mass, float dt) noexcept
{
	// @Note: what i"m calling mass is called "gravity" in the CK - appears to be a scalar on gravity
	// There is ALSO a gravityFactor in the nif file for the arrow

	// From the iron arrow nif file
	// Until I can figure out a way to read these values at runtime, assume they don't change
	constexpr auto linDamp = 0.099609f;
	constexpr auto gravityFactor = 1.0f;

	const auto magic = glm::vec3{ 0.0f, 0.0f, 59.0f };
	const glm::vec3 gravityVector = gravity * gravityFactor * mass * magic;
	const auto step = gravityVector -(vel * linDamp);

	vel += step * dt;
	position += vel * dt;
}

glm::vec3 Crosshair::Manager::ComputeProjectileVelocityVector(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const TESAmmo* ammo, const glm::vec2& aimRotation) noexcept
{
	// Read the tilt angle
	float tilt = 0.0f;
	if (GameState::IsUsingBow(player) || GameState::IsUsingCrossbow(player)) {
		static auto arrowTilt = (*g_iniSettingCollection)->Get("f3PArrowTiltUpAngle:Combat");
		tilt = glm::radians(arrowTilt->data.f32);
	}

	// Get the rotation for the arrow
	auto arrowRot = aimRotation;
	// Offset with tilt angle and invert
	arrowRot.x -= tilt;
	arrowRot.x *= -1;

	// Read required metrics for the shot

	// Replicates the function at 42537
	//const auto s2 = Offsets::Get<GetAFloat>(42537)(arrow);
	const auto s2 = [](const TESObjectWEAP* wep) {
		if (!wep) return 1.0f;

		auto speed = wep->gameData.speed;
		const auto DAT_141de0da8 = *Offsets::Get<float*>(505064);
		const auto DAT_141de0dc0 = *Offsets::Get<float*>(505066);
		const auto DAT_142f01438 = *Offsets::Get<float*>(515530);

		if (DAT_141de0da8 <= speed)
			speed = DAT_141de0da8;

		if (speed <= DAT_141de0dc0)
			speed = DAT_141de0dc0;

		return (DAT_141de0dc0 - speed) * DAT_142f01438 + 1.0f;
	}(GameState::GetEquippedWeapon(player));

	// Replicates the function at 42536
	// const auto power = Offsets::Get<GetAFloat>(42536)(arrow);
	const auto power = [](float unk188_ArrowDrawTimer) {
		// @Note: The parameter is projectile->unk188
		const float fVar1 = unk188_ArrowDrawTimer; // FUN_14074dc80(arrowProjectile);

		const auto DAT_141de0df0 = *Offsets::Get<float*>(505070);
		const auto DAT_141de0e08 = *Offsets::Get<float*>(505072);

		return ((fVar1 - DAT_141de0df0) / (1.0f - DAT_141de0df0)) * (1.0f - DAT_141de0e08) + DAT_141de0e08;
	}(GameState::GetCurrentBowDrawTimer(player));

	const auto speed = ammo->settings.projectile->data.speed;
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

bool Crosshair::Manager::ProjectilePredictionCurve(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const glm::vec2& aimRotation, const glm::vec3& startPos, glm::vec3& hitPos, bool& hitCharacter) noexcept
{
	// Get the ammo we are using
	const auto ammo = GameState::GetCurrentAmmo(player);
	if (ammo == nullptr) return false;

	const auto config = Config::GetCurrentConfig();
	const auto gravity = Physics::GetGravityVector(player);
	const auto mass = *reinterpret_cast<float*>(&ammo->settings.projectile->data.unk04);

	// Compute impulse
	glm::vec3 velocityVector = ComputeProjectileVelocityVector(player, camera, ammo, aimRotation);

	// Max number of segments we want to simulate
	constexpr auto segCount = 128;
	constexpr float timeStep = 1.0f / 20.0f; // The slower the timestep, the more we decimate the arc
	glm::vec3 lastPos = startPos;
	glm::vec3 curPos = startPos;

	uint8_t entries = 0;
	eastl::array<eastl::tuple<glm::vec3, glm::vec3>, segCount> points;

	bool hit = false;
	for (auto i = 0; i < segCount; i++) {
		TickProjectilePath(curPos, velocityVector, gravity, mass, timeStep);
		const auto origin = glm::vec4(lastPos.x, lastPos.y, lastPos.z, 0.0f);
		const auto endPoint = glm::vec4(curPos.x, curPos.y, curPos.z, 0.0f);
		const auto rayLength = glm::length(endPoint - origin);
		const auto rayNormal = glm::normalize(endPoint - origin);
		const auto ray = rayNormal * rayLength;
		const auto result = Raycast::hkpCastRay(origin, origin + ray);

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

NiAVObject* Crosshair::Manager::FindArrowNode(const PlayerCharacter* player) const noexcept {
	auto handNode = DYNAMIC_CAST(player->loadedState->node->GetObjectByName(&Strings.weapon.data), NiAVObject, NiNode);
	if (handNode && handNode->m_children.m_size > 0 && handNode->m_children.m_data) {
		auto arrow = handNode->GetObjectByName(&Strings.arrowName.data);
		if (arrow) return arrow;
	}

	handNode = DYNAMIC_CAST(player->loadedState->node->GetObjectByName(&Strings.rmag.data), NiAVObject, NiNode);
	if (handNode && handNode->m_children.m_size > 0 && handNode->m_children.m_data) {
		auto arrow = handNode->GetObjectByName(&Strings.arrowName.data);
		if (arrow) return arrow;
	}

	handNode = DYNAMIC_CAST(player->loadedState->node->GetObjectByName(&Strings.lmag.data), NiAVObject, NiNode);
	if (handNode && handNode->m_children.m_size > 0 && handNode->m_children.m_data) {
		auto arrow = handNode->GetObjectByName(&Strings.arrowName.data);
		if (arrow) return arrow;
	}

	return nullptr;
}

void Crosshair::Manager::UpdateCrosshairPosition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const glm::vec2& aimRotation, mmath::NiMatrix44& worldToScaleform) noexcept
{
	if (Messaging::SmoothCamAPIV1::GetInstance()->IsCrosshairTaken()) return;
	if (!player->loadedState || !player->loadedState->node) return;

	const auto config = Config::GetCurrentConfig();
	auto maxRayLength = 8000.0f;
	glm::vec3 hitPos = { 0, 0, 0 };
	float rayLength = 0.0f;
	bool hit = false;
	bool hitCharacter = false;

	if (GameState::IsBowDrawn(player) && Strings.weapon.data && Strings.arrowName.data) {
		const auto arrow = FindArrowNode(player);
		if (arrow) {
			const auto pos = glm::vec3{
				arrow->m_worldTransform.pos.x,
				arrow->m_worldTransform.pos.y,
				arrow->m_worldTransform.pos.z
			};

			// Now select the method to use
			if (config->useArrowPrediction) {
				maxRayLength = config->maxArrowPredictionRange;
				if (ProjectilePredictionCurve(player, camera, aimRotation, pos, hitPos, hitCharacter)) {
					hit = true;
					rayLength = glm::length(hitPos - pos);
				}
			} else {
				// Classic method
				float fac = 0.0f;
				if (GameState::IsUsingBow(player) || GameState::IsUsingCrossbow(player)) {
					static auto arrowTilt = (*g_iniSettingCollection)->Get("f3PArrowTiltUpAngle:Combat");
					fac = glm::radians(arrowTilt->data.f32) * 0.5f;
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
	} else if (GameState::IsMagicDrawn(player) && Strings.magic.data) {
		NiPoint3 niOrigin = { 0.01f, 0.01f, 0.01f };
		glm::vec3 normal = { 0.0f, 1.00f, 0.0f };

		const auto handNode = DYNAMIC_CAST(player->loadedState->node->GetObjectByName(&Strings.magic.data), NiAVObject, NiNode);
		if (handNode)
			niOrigin = { handNode->m_worldTransform.pos.x, handNode->m_worldTransform.pos.y, handNode->m_worldTransform.pos.z };
		normal = GetCrosshairTargetNormal(aimRotation);

		// Cast the aim ray
		const auto origin = glm::vec4(niOrigin.x, niOrigin.y, niOrigin.z, 0.0f);
		const auto ray = glm::vec4(normal.x, normal.y, normal.z, 0.0f) * maxRayLength;
		const auto result = Raycast::hkpCastRay(origin, origin + ray);
		hit = result.hit;
		hitPos = result.hitPos;
		rayLength = result.rayLength;
		hitCharacter = result.hitCharacter != nullptr;
	}

	// Now set the crosshair
	glm::vec2 crosshairSize(baseCrosshairData.xScale, baseCrosshairData.yScale);
	glm::vec2 crosshairPos(baseCrosshairData.xCenter, baseCrosshairData.yCenter);
	if (hit) {
		auto port = NiRect<float>();
		auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
		if (menu && menu->view) {
			const auto rect = menu->view->GetVisibleFrameRect();
			port.m_left = rect.left;
			port.m_right = rect.right;
			port.m_top = rect.bottom;
			port.m_bottom = rect.top;
		}

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
				mmath::half_pi - aimRotation.x,
				aimRotation.y * -1.0f,
				0.0f
			});

			return;
		}
		
		auto pt = NiPoint3(
			hitPos.x,
			hitPos.y,
			hitPos.z
		);

		auto rangeScalar = glm::clamp((maxRayLength - rayLength) / maxRayLength, 0.0f, 1.0f);
		auto sz = mmath::Remap(rangeScalar, 0.0f, 1.0f, config->crosshairMinDistSize, config->crosshairMaxDistSize);
		crosshairSize = { sz, sz };

		// Use the HUD crosshair
		glm::vec3 screen = {};
		(*WorldPtToScreenPt3_Internal)(
			reinterpret_cast<float*>(&worldToScaleform),
			&port, &pt,
			&screen.x, &screen.y, &screen.z, 9.99999975e-06
		);

		if (hitCharacter)
			crosshairSize += config->crosshairNPCHitGrowSize * rangeScalar;

		crosshairPos = {
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
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (!menu || !menu->view) return;

	GFxValue va;
	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._x");
	baseCrosshairData.xOff = va.GetNumber();

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._y");
	baseCrosshairData.yOff = va.GetNumber();

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._width");
	baseCrosshairData.xScale = va.GetNumber();

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.CrosshairInstance._height");
	baseCrosshairData.yScale = va.GetNumber();

	const auto rect = menu->view->GetVisibleFrameRect();
	baseCrosshairData.xCenter = mmath::Remap(0.5f, 0.0f, 1.0f, rect.left, rect.right);
	baseCrosshairData.yCenter = mmath::Remap(0.5f, 0.0f, 1.0f, rect.top, rect.bottom);

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.StealthMeterInstance._x");
	baseCrosshairData.stealthXOff = va.GetNumber();

	menu->view->GetVariable(&va, "_root.HUDMovieBaseInstance.StealthMeterInstance._y");
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

	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	GFxValue instance;
	if (menu && menu->view && menu->view->GetVariable(&instance, "_root.HUDMovieBaseInstance.Crosshair")) {
		const auto rect = menu->view->GetVisibleFrameRect();
		const auto half_x = pos.x -
			((static_cast<double>(rect.right) + static_cast<double>(rect.left)) * 0.5);
		const auto half_y = pos.y -
			((static_cast<double>(rect.bottom) + static_cast<double>(rect.top)) * 0.5);

		const auto x = half_x + baseCrosshairData.xOff;
		const auto y = half_y + baseCrosshairData.yOff;

		GFxValue member;
		if (instance.GetMember("_x", &member)) {
			member.SetNumber(x);
			instance.SetMember("_x", &member);
		}

		if (instance.GetMember("_y", &member)) {
			member.SetNumber(y);
			instance.SetMember("_y", &member);
		}

		menu->view->SetVariable("_root.HUDMovieBaseInstance.Crosshair", &instance, 2);
		currentCrosshairData.position = pos;
		g_crosshairData.position = pos;
		g_crosshairData.ofs = { x, y };
	}
}

void Crosshair::Manager::SetStealthMeterPosition(const glm::vec2& pos) noexcept {
	if (!IsCrosshairDataValid()) return;
	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	GFxValue instance;
	if (menu && menu->view && menu->view->GetVariable(&instance, "_root.HUDMovieBaseInstance.StealthMeterInstance")) {
		GFxValue member;
		if (instance.GetMember("_x", &member)) {
			member.SetNumber(baseCrosshairData.stealthXOff + static_cast<double>(pos.x));
			instance.SetMember("_x", &member);
		}

		if (instance.GetMember("_y", &member)) {
			member.SetNumber(baseCrosshairData.stealthYOff + static_cast<double>(pos.y));
			instance.SetMember("_y", &member);
		}

		menu->view->SetVariable("_root.HUDMovieBaseInstance.StealthMeterInstance", &instance, 2);
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
}

void Crosshair::Manager::SetCrosshairSize(const glm::dvec2& size) noexcept {
	if (!IsCrosshairDataValid()) return;
	if (currentCrosshairData.scale == size) return;
	if (!Config::GetCurrentConfig()->enableCrosshairSizeManip) return;

	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	GFxValue instance;
	if (menu && menu->view && menu->view->GetVariable(&instance, "_root.HUDMovieBaseInstance.Crosshair")) {
		GFxValue member;
		if (instance.GetMember("_width", &member)) {
			member.SetNumber(size.x);
			instance.SetMember("_width", &member);
		}

		if (instance.GetMember("_height", &member)) {
			member.SetNumber(size.y);
			instance.SetMember("_height", &member);
		}

		menu->view->SetVariable("_root.HUDMovieBaseInstance.Crosshair", &instance, 2);
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

	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue result;
		GFxValue args[2];
		args[0].SetString("SetCrosshairEnabled");
		args[1].SetBool(enabled);
		menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 2);
		
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

void Crosshair::Manager::Update(PlayerCharacter* player, CorrectedPlayerCamera* camera) noexcept {
	if (!IsCrosshairDataValid()) return;

	if (!Messaging::SmoothCamAPIV1::GetInstance()->IsCrosshairTaken())
		if (Messaging::SmoothCamAPIV1::GetInstance()->CrosshairDirty()) {
			ResetCrosshair();
			Messaging::SmoothCamAPIV1::GetInstance()->ClearCrosshairDirtyFlag();
		}

	if (Messaging::SmoothCamAPIV1::GetInstance()->IsStealthMeterTaken()) return;
	if (Messaging::SmoothCamAPIV1::GetInstance()->StealthMeterDirty()) {
		ResetStealthMeter();
		Messaging::SmoothCamAPIV1::GetInstance()->ClearStealthMeterDirtyFlag();
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
			currentCrosshairData.stealthMeterMutated = false;
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
		currentCrosshairData.stealthMeterMutated = false;
	}
}

void Crosshair::Manager::Render(Render::D3DContext& ctx, const glm::vec3& cameraPosition, const glm::vec2& cameraRotation,
	const NiFrustum& frustum) noexcept
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

		renderables.curCrosshair->Render(
			ctx, renderables.cbufPerFrameStaging.curTime, GameTime::GetFrameDelta(), config->worldCrosshairDepthTest
		);
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