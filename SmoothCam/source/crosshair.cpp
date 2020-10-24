#include "crosshair.h"

#include "crosshair/skyrim.h"
#include "crosshair/dot.h"

double CurTime() noexcept;
double CurQPC() noexcept;
double GetFrameDelta() noexcept;
double GetQPCDelta() noexcept;

Crosshair::Manager::Manager() {
	weapon = "WEAPON";

	ReadInitialCrosshairInfo();

	if (!Render::HasContext()) return;
	auto& ctx = Render::GetContext();

	// Per-object data, changing each draw call (model)
	Render::CBufferCreateInfo perObj;
	perObj.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	perObj.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	perObj.size = sizeof(decltype(renderables.cbufPerObjectStaging));
	perObj.initialData = &renderables.cbufPerObjectStaging;
	renderables.cbufPerObject = std::make_shared<Render::CBuffer>(perObj, ctx);

	// Per-frame data, shared among many objects (view, projection)
	Render::CBufferCreateInfo perFrane;
	perFrane.bufferUsage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	perFrane.cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	perFrane.size = sizeof(decltype(renderables.cbufPerFrameStaging));
	perFrane.initialData = &renderables.cbufPerFrameStaging;
	renderables.cbufPerFrame = std::make_shared<Render::CBuffer>(perFrane, ctx);

	// Create our line drawer for the crosshair tail
	renderables.tailDrawer = std::make_unique<Render::LineDrawer>(ctx);
}

Crosshair::Manager::~Manager() {
	renderables.release();
}

glm::vec3 Crosshair::Manager::GetCrosshairTargetNormal(const glm::vec2& aimRotation, float pitchMod) {
	return mmath::GetViewVector(
		glm::vec3(0.0, 1.0, 0.0),
		aimRotation.x - pitchMod,
		aimRotation.y
	);
}

void Crosshair::Manager::TickProjectilePath(glm::vec3& position, glm::vec3& vel, float gravity, float dt) noexcept {
	// Not really sure what skyrim's gravity actually is, but this works (close enough)
	constexpr auto magic = 650.0f;
	const auto velNormal = glm::normalize(vel);
	glm::vec3 gravityVector = { 0, 0, gravity * magic };
	vel -= gravityVector * dt;
	position += vel * dt;
}

glm::vec3 Crosshair::Manager::ComputeProjectileVelocityVector(PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const TESAmmo* ammo, float gravity, const glm::vec2& aimRotation) noexcept
{
	// Read the tilt angle
	float tilt = 0.0f;
	if (GameState::IsUsingCrossbow(player))
		tilt = glm::radians(Config::GetGameConfig()->f3PBoltTiltUpAngle);
	else if (GameState::IsUsingBow(player))
		tilt = glm::radians(Config::GetGameConfig()->f3PArrowTiltUpAngle);

	// Get the rotation for the arrow
	auto arrowRot = aimRotation;
	// Offset with tilt angle and invert
	arrowRot.x -= tilt;
	arrowRot.x *= -1;

	// Read required metrics for the shot
	//const auto s2 = Offsets::Get<GetAFloat>(42537)(arrow);
	const auto s2 = GameState::GetEquippedWeapon(player)->gameData.speed;
	// (DAT_141de0dc0 - param_2) * _DAT_142f01438 + 1.00000000;
	static auto pF1 = Offsets::Get<float*>(505066);
	static auto pF2 = Offsets::Get<float*>(515530);
	auto f1 = *pF1;
	auto f2 = *pF2;
	const auto launchSpeed = (f1 - s2) * f2 + 1.0f;
	const auto speed = ammo->settings.projectile->data.speed;

	//const auto power = Offsets::Get<GetAFloat>(42536)(arrow);
	// Let's assume a full power shot
	constexpr auto power = 1.0f;
	// arrowProjectile->arrowUnk18C
	// Appears to always be one
	constexpr auto arrowUnk18C = 1.0f;

	// Now calculate the initial velocity vector for the arrow if we were to fire it right now
	const auto arrowFireSpeed = speed * power;
	const auto velScalar = launchSpeed * arrowFireSpeed * arrowUnk18C;
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

bool Crosshair::Manager::ProjectilePredictionCurve(PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const glm::vec2& aimRotation, const glm::vec3& startPos, glm::vec3& hitPos) noexcept
{
	// Get the ammo we are using
	typedef TESAmmo*(__thiscall PlayerCharacter::* GetAmmo)();
	const auto ammo = (player->*reinterpret_cast<GetAmmo>(&PlayerCharacter::Unk_9E))();
	if (ammo == nullptr) return false;

	// Compute impulse
	const auto gravity = *reinterpret_cast<float*>(&ammo->settings.projectile->data.unk04);
	glm::vec3 velocityVector = ComputeProjectileVelocityVector(player, camera, ammo, gravity, aimRotation);

	const auto config = Config::GetCurrentConfig();

	// And simulate the arc, trace in discrete slices

	// Max number of segments we want to simulate
	constexpr auto segCount = 128;
	constexpr float timeStep = 1.0f / 16.0f; // The slower the timestep, the more we decimate the arc
	glm::vec3 lastPos = startPos;
	glm::vec3 curPos = startPos;

	uint8_t entries = 0;
	std::array<std::tuple<glm::vec3, glm::vec3>, segCount> points;

	bool hit = false;
	for (auto i = 0; i < segCount; i++) {
		TickProjectilePath(curPos, velocityVector, gravity, timeStep);
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
					l1 * Render::RenderScale,
					{
						col.r,
						col.g,
						col.b,
						(1.0f - glm::clamp(alpha, 0.0f, 1.0f)) * col.a
					}
				),
				Render::Point(
					l2 * Render::RenderScale,
					{
						col.r,
						col.g,
						col.b,
						(1.0f - glm::clamp(alpha2, 0.0f, 1.0f)) * col.a
					}
				)
			);
		}
	}

	return hit;
}

void Crosshair::Manager::UpdateCrosshairPosition(PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const glm::vec2& aimRotation, mmath::NiMatrix44& worldToScaleform)
{
	if (!player->loadedState || !player->loadedState->node) return;

	const auto config = Config::GetCurrentConfig();
	auto maxRayLength = 8000.0f;
	glm::vec3 hitPos = { 0, 0, 0 };
	float rayLength = 0.0f;
	bool hit = false;

	if (GameState::IsBowDrawn(player) && weapon.data) {
		const auto handNode = DYNAMIC_CAST(player->loadedState->node->GetObjectByName(&weapon.data), NiAVObject, NiNode);

		if (handNode && handNode->m_children.m_size > 0 && handNode->m_children.m_data) {
			const auto arrow = handNode->m_children.m_data[0];
			if (arrow != nullptr) {
				const auto pos = glm::vec3{
					arrow->m_worldTransform.pos.x,
					arrow->m_worldTransform.pos.y,
					arrow->m_worldTransform.pos.z
				};

				// Now select the method to use
				if (config->useArrowPrediction) {
					maxRayLength = config->maxArrowPredictionRange;
					if (ProjectilePredictionCurve(player, camera, aimRotation, pos, hitPos)) {
						hit = true;
						rayLength = glm::length(hitPos - pos);
					}
				} else {
					// Classic method
					float fac = 0.0f;
					if (GameState::IsUsingCrossbow(player)) {
						fac = glm::radians(Config::GetGameConfig()->f3PBoltTiltUpAngle) * 0.5f;
					} else if (GameState::IsUsingBow(player)) {
						fac = glm::radians(Config::GetGameConfig()->f3PArrowTiltUpAngle) * 0.5f;
					}
					const auto n = GetCrosshairTargetNormal(aimRotation, fac);

					auto origin = glm::vec4(pos.x, pos.y, pos.z, 0.0f);
					auto ray = glm::vec4(n.x, n.y, n.z, 0.0f) * maxRayLength;
					const auto result = Raycast::hkpCastRay(origin, origin + ray);
					hit = result.hit;
					hitPos = result.hitPos;
					rayLength = result.rayLength;
				}
			}
		}
	} else if (GameState::IsMagicDrawn(player) && weapon.data) {
		NiPoint3 niOrigin = { 0.01f, 0.01f, 0.01f };
		glm::vec3 normal = { 0.0f, 1.00f, 0.0f };

		const auto handNode = DYNAMIC_CAST(player->loadedState->node->GetObjectByName(&weapon.data), NiAVObject, NiNode);
		if (handNode)
			niOrigin = { handNode->m_worldTransform.pos.x,handNode->m_worldTransform.pos.y,handNode->m_worldTransform.pos.z };
		normal = GetCrosshairTargetNormal(aimRotation);

		// Cast the aim ray
		auto origin = glm::vec4(niOrigin.x, niOrigin.y, niOrigin.z, 0.0f);
		auto ray = glm::vec4(normal.x, normal.y, normal.z, 0.0f) * maxRayLength;
		const auto result = Raycast::hkpCastRay(origin, origin + ray);
		hit = result.hit;
		hitPos = result.hitPos;
		rayLength = result.rayLength;
	}

	// Now set the crosshair
	glm::vec2 crosshairSize(baseCrosshairData.xScale, baseCrosshairData.yScale);
	glm::vec2 crosshairPos(baseCrosshairData.xCenter, baseCrosshairData.yCenter);
	if (hit) {
		auto port = NiRect<float>();
		auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
		if (menu && menu->view) {
			auto rect = menu->view->GetVisibleFrameRect();
			port.m_left = rect.left;
			port.m_right = rect.right;
			port.m_top = rect.bottom;
			port.m_bottom = rect.top;
		}

		auto pt = NiPoint3(
			hitPos.x,
			hitPos.y,
			hitPos.z
		);

		auto rangeScalar = glm::clamp((maxRayLength - rayLength) / maxRayLength, 0.0f, 1.0f);
		auto sz = mmath::Remap(rangeScalar, 0.0f, 1.0f, config->crosshairMinDistSize, config->crosshairMaxDistSize);
		crosshairSize = { sz, sz };

		// Until we can actually detect this, forget about it
		//if (result.hitCharacter)
		//	crosshairSize += config->crosshairNPCHitGrowSize * rangeScalar;

		// Flag our crosshair for drawing, if we have one
		if (config->useWorldCrosshair && renderables.curCrosshair) {
			SetCrosshairEnabled(false);

			// Give us a bit of breathing room if using the bow - we don't want to draw the crosshair
			// on the player's back for the first few frames.
			if (GameState::IsBowDrawn(player) && rayLength <= 15.0f) return;

			renderables.drawCrosshair = true;
			renderables.curCrosshair->SetPosition(hitPos);

			// We want the crosshair to face the player
			const glm::vec3 bilboard = {
				mmath::half_pi - aimRotation.x,
				aimRotation.y * -1.0f,
				0.0f
			};

			renderables.curCrosshair->SetRotation(bilboard);
			return;

		} else {
			// Use the HUD crosshair
			glm::vec3 screen = {};
			(*WorldPtToScreenPt3_Internal)(
				reinterpret_cast<float*>(&worldToScaleform),
				&port, &pt,
				&screen.x, &screen.y, &screen.z, 9.99999975e-06
			);

			crosshairPos = {
				screen.x,
				screen.y
			};
		}
	}

	SetCrosshairEnabled(true);
	SetCrosshairPosition(crosshairPos);
	SetCrosshairSize(crosshairSize);
}

void Crosshair::Manager::ReadInitialCrosshairInfo() {
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

	auto rect = menu->view->GetVisibleFrameRect();
	baseCrosshairData.xCenter = mmath::Remap(0.5f, 0.0f, 1.0f, rect.left, rect.right);
	baseCrosshairData.yCenter = mmath::Remap(0.5f, 0.0f, 1.0f, rect.top, rect.bottom);

	baseCrosshairData.captured = true;

	currentCrosshairData.position = {
		baseCrosshairData.xOff,
		baseCrosshairData.yOff
	};
	currentCrosshairData.scale = {
		baseCrosshairData.xScale,
		baseCrosshairData.yScale
	};
	currentCrosshairData.enabled = true;
}

void Crosshair::Manager::SetCrosshairPosition(const glm::dvec2& pos) {
	if (currentCrosshairData.position == pos) return;

	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		const auto rect = menu->view->GetVisibleFrameRect();
		const auto half_x = pos.x - ((rect.right + rect.left) * 0.5);
		const auto half_y = pos.y - ((rect.bottom + rect.top) * 0.5);

		const auto x = half_x + baseCrosshairData.xOff;
		const auto y = half_y + baseCrosshairData.yOff;

		GFxValue va;
		va.SetNumber(x);
		menu->view->SetVariable("_root.HUDMovieBaseInstance.CrosshairInstance._x", &va, 0);
		va.SetNumber(y);
		menu->view->SetVariable("_root.HUDMovieBaseInstance.CrosshairInstance._y", &va, 0);

		currentCrosshairData.position = pos;
	}
}

void Crosshair::Manager::CenterCrosshair() {
	SetCrosshairPosition({
		baseCrosshairData.xCenter,
		baseCrosshairData.yCenter
	});
}

void Crosshair::Manager::SetCrosshairSize(const glm::dvec2& size) {
	if (currentCrosshairData.scale == size) return;
	if (!Config::GetCurrentConfig()->enableCrosshairSizeManip) return;

	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue va;
		va.SetNumber(static_cast<double>(size.x));
		menu->view->SetVariable("_root.HUDMovieBaseInstance.Crosshair._width", &va, 0);
		va.SetNumber(static_cast<double>(size.y));
		menu->view->SetVariable("_root.HUDMovieBaseInstance.Crosshair._height", &va, 0);

		currentCrosshairData.scale = size;
	}
}

void Crosshair::Manager::SetDefaultSize() {
	SetCrosshairSize({
		baseCrosshairData.xScale,
		baseCrosshairData.yScale
	});
}

void Crosshair::Manager::SetCrosshairEnabled(bool enabled) {
	if (currentCrosshairData.enabled == enabled) return;

	auto menu = MenuManager::GetSingleton()->GetMenu(&UIStringHolder::GetSingleton()->hudMenu);
	if (menu && menu->view) {
		GFxValue result;
		GFxValue args[2];
		args[0].SetString("SetCrosshairEnabled");
		args[1].SetBool(enabled);
		menu->view->Invoke("call", &result, static_cast<GFxValue*>(args), 2);
		
		currentCrosshairData.enabled = enabled;
	}
}

void Crosshair::Manager::Set3DCrosshairType(Config::CrosshairType type) {
	if (!Render::HasContext()) return;
	if (renderables.crosshairType == type) return;

	// Crosshair either changed or this is the first time running
	switch (type) {
		case Config::CrosshairType::Skyrim:
			renderables.curCrosshair = std::make_unique<Crosshair::Skyrim>();
			break;
		case Config::CrosshairType::Dot:
			renderables.curCrosshair = std::make_unique<Crosshair::Dot>();
			break;
		default:
			renderables.curCrosshair = std::make_unique<Crosshair::Skyrim>();
			break;
	}

	renderables.curCrosshair->Create3D(Render::GetContext(), renderables.cbufPerObject);
	renderables.crosshairType = type;
}

void Crosshair::Manager::Render(Render::D3DContext& ctx, const glm::vec3& cameraPosition, const glm::vec2& cameraRotation,
	const NiFrustum& frustum) noexcept
{
	const auto config = Config::GetCurrentConfig();

	// Update view, projection and common per frame data
	const auto matProj = Render::GetProjectionMatrix(frustum);
	const auto matView = Render::BuildViewMatrix(cameraPosition, cameraRotation);
	renderables.cbufPerFrameStaging.matProjView = matProj * matView;

	renderables.cbufPerFrameStaging.curTime = static_cast<float>(CurTime());
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

	if (renderables.arrowTailSegments.size() > 0) {
		renderables.tailDrawer->Submit(renderables.arrowTailSegments);
		renderables.arrowTailSegments.clear();
	}

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
		}

		renderables.curCrosshair->SetScale({
			pScale * 0.001f * s * 0.04f,
			pScale * 0.001f * s * 0.04f,
			1.0f
		});

		renderables.curCrosshair->Render(
			ctx, renderables.cbufPerFrameStaging.curTime, GetFrameDelta(), config->worldCrosshairDepthTest
		);
		renderables.drawCrosshair = false;
	}
}