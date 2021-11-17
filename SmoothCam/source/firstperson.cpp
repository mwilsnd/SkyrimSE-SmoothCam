#include "firstperson.h"

#ifdef DEVELOPER
extern Offsets* g_Offsets;

//1406a1820:39401
typedef uintptr_t(*SwitchSkeleton)(RE::Actor*, byte);
static eastl::unique_ptr<TypedDetour<SwitchSkeleton>> detSwitchSkeleton;
static bool tpSkeletonVisible = true;
uintptr_t mSwitchSkeleton(RE::Actor* actor, byte param_2) {
	tpSkeletonVisible = !param_2;
	return detSwitchSkeleton->GetBase()(actor, param_2);
}

static eastl::map<ID3D11RasterizerState*, winrt::com_ptr<ID3D11RasterizerState>>* rasterCache = nullptr;
static eastl::vector<RE::NiAVObject*>* clipObjects = nullptr;
static eastl::vector<RE::NiAVObject*>* shadowOnlyObjects = nullptr;
static bool inFPV = false;

typedef void(*RenderStuff)(Render::BatchRenderCommand* cmd, uint32_t id, bool alphaBlend, uint32_t passFlags);
static eastl::unique_ptr<TypedDetour<RenderStuff>> detRenderStuff;
void mRenderStuff(Render::BatchRenderCommand* cmd, uint32_t id, bool alphaBlend, uint32_t passFlags) {
	if (!inFPV || !cmd->geometry || !shadowOnlyObjects || !clipObjects || !rasterCache)
		return detRenderStuff->GetBase()(cmd, id, alphaBlend, passFlags);

	const auto bits = eastl::bitset<32>(passFlags);
	const bool isShadowCascade = bits[10];
	const auto isZPass = bits[0] && bits[5];
	const auto isGBufferPass = bits[6];
	const auto isUnkPass = !isZPass && !isGBufferPass && bits[0];

	// Geometry we only want in the shadow maps
	if (!isShadowCascade || isUnkPass)
		for (const auto& geo : *shadowOnlyObjects)
			if (geo == cmd->geometry)
				return;

	for (const auto& geo : *clipObjects) {
		if (geo != cmd->geometry) continue;

		if (!isZPass && !isGBufferPass) {
			if (isShadowCascade)
				return detRenderStuff->GetBase()(cmd, id, alphaBlend, passFlags);

			//if (isUnkPass)
			//	return detRenderStuff->GetBase()(cmd, id, alphaBlend, passFlags);
		}

		auto& ctx = Render::GetContext();
		winrt::com_ptr<ID3D11RasterizerState> gameState;
		ctx.context->RSGetState(gameState.put());


		auto it = rasterCache->find(gameState.get());
		if (it == rasterCache->end()) {
			D3D11_RASTERIZER_DESC desc;
			gameState->GetDesc(&desc);
			desc.DepthClipEnable = false;
			//desc.DepthBias += isZPass ? 10000 : -10000;
			//desc.DepthBiasClamp = 0.0f;
			//desc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;

			winrt::com_ptr<ID3D11RasterizerState> clipState;
			if (!SUCCEEDED(ctx.device->CreateRasterizerState(&desc, clipState.put())))
				return detRenderStuff->GetBase()(cmd, id, alphaBlend, passFlags);

			rasterCache->insert({ gameState.get(), clipState });
			if (!isUnkPass)
				ctx.context->RSSetState(clipState.get());
		} else {
			ctx.context->RSSetState(it->second.get());
		}

		winrt::com_ptr<ID3D11DepthStencilState> ds;
		UINT ref;
		ctx.context->OMGetDepthStencilState(ds.put(), &ref);
		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ds->GetDesc(&dsDesc);

		winrt::com_ptr<ID3D11DepthStencilState> dsFlicker;
		if (!isUnkPass && dsDesc.DepthFunc == D3D11_COMPARISON_EQUAL) {
			dsDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
			ctx.device->CreateDepthStencilState(&dsDesc, dsFlicker.put());
			ctx.context->OMSetDepthStencilState(dsFlicker.get(), ref);
		}

		detRenderStuff->GetBase()(cmd, id, alphaBlend, passFlags);
		ctx.context->RSSetState(gameState.get());
		if (dsFlicker)
			ctx.context->OMSetDepthStencilState(ds.get(), ref);

		return;
	}

	detRenderStuff->GetBase()(cmd, id, alphaBlend, passFlags);
}
#endif

Camera::Firstperson::Firstperson(Camera* baseCamera) : ICamera(baseCamera, CameraID::Firstperson) {
#ifdef DEVELOPER
	config = Config::GetCurrentConfig();

	rasterCache = &rasterDepthClipStates;
	clipObjects = &wantClipNodes;
	shadowOnlyObjects = &shadowsOnly;

	// Game is switching player skeleton
	detSwitchSkeleton = eastl::make_unique<TypedDetour<SwitchSkeleton>>(
		g_Offsets->SwitchSkeleton,
		mSwitchSkeleton
	);
	detSwitchSkeleton->Attach();

	// Rendering stuffs
	detRenderStuff = eastl::make_unique<TypedDetour<RenderStuff>>(
		g_Offsets->RenderStuff,
		mRenderStuff
	);
	detRenderStuff->Attach();
#endif
}

Camera::Firstperson::~Firstperson() {}

#ifdef DEVELOPER
void Camera::Firstperson::OnBegin(RE::PlayerCharacter* player, RE::PlayerCamera*, ICamera*) noexcept {
#else
void Camera::Firstperson::OnBegin(RE::PlayerCharacter*, RE::PlayerCamera*, ICamera*) noexcept {
#endif
#ifdef DEVELOPER
	inFPV = true;
	if (m_camera->currentFocusObject == player)
		ToggleThirdpersonSkeleton(true);
#endif
}

#ifdef DEVELOPER
void Camera::Firstperson::OnEnd(RE::PlayerCharacter* player, RE::PlayerCamera*, ICamera*) noexcept {
#else
void Camera::Firstperson::OnEnd(RE::PlayerCharacter*, RE::PlayerCamera*, ICamera*) noexcept {
#endif
#ifdef DEVELOPER
	inFPV = false;
	if (m_camera->currentFocusObject == player) {
		ToggleThirdpersonSkeleton(true);
	}
#endif
}

bool Camera::Firstperson::OnPreGameUpdate(RE::PlayerCharacter*, RE::PlayerCamera*,
	RE::BSTSmartPointer<RE::TESCameraState>&)
{
	return false;
}

#ifdef DEVELOPER
void Camera::Firstperson::OnUpdateCamera(RE::PlayerCharacter* player, RE::PlayerCamera* camera,
	RE::BSTSmartPointer<RE::TESCameraState>&)
{
#else
void Camera::Firstperson::OnUpdateCamera(RE::PlayerCharacter*, RE::PlayerCamera*,
	RE::BSTSmartPointer<RE::TESCameraState>&)
{
#endif
#ifdef DEVELOPER
	if (!camera->currentState || camera->currentState->id != RE::CameraState::kFirstPerson) return;

	auto state = reinterpret_cast<RE::FirstPersonState*>(camera->currentState.get());
	if (!state->firstPersonCameraObj) return;
	if (!m_camera->currentFocusObject->loadedData || !m_camera->currentFocusObject->loadedData->data3D) return;

	UpdateClipableWeaponNodes(player);
	UpdateShadowOnlyNodes(player);

	// Make sure the thirdperson skeleton is visible
	if (!tpSkeletonVisible)
		ToggleThirdpersonSkeleton(true);

	// Position & trackir stuff
	auto refPos = glm::vec3{
		state->firstPersonCameraObj->world.translate.x,
		state->firstPersonCameraObj->world.translate.y,
		state->firstPersonCameraObj->world.translate.z
	};

	// Extract camera rotation
	auto rot = mmath::NiMatrixToEuler(m_camera->cameraNi->world.rotate);
	// Current TrackIR data
	const auto& tracking = m_camera->trackIRData;

	RE::BSFixedString head = "NPC Head [Head]";
	auto node = m_camera->currentFocusObject->loadedData->data3D->GetObjectByName(head);
	if (node) {
		glm::vec3 headPos = {
			node->world.translate.x,
			node->world.translate.y,
			node->world.translate.z
		};

		// And player basis vectors
		auto obj = m_camera->currentFocusObject;
		glm::vec3 f, s, u, coef;
		mmath::DecomposeToBasis(
			refPos,
			{ obj->GetAngleX(), obj->GetAngleY(), obj->GetAngleZ() },
			f, s, u, coef
		);

		// Position
		// @TODO: This is kind of crap, can we get some form of body IK going here?
		constexpr auto scale = 0.01f;
		glm::vec3 trackPos = tracking.pos * scale;
		headPos += (f * -trackPos.x) + (s * -trackPos.z) + (u * trackPos.y);

		// Now yaw locked offsets
		mmath::DecomposeToBasis(
			refPos,
			{ 0.0f, 0.0f, obj->GetAngleZ() },
			f, s, u, coef
		);

		// Add our fixed offset
		headPos += (u * 5.0f) + (s * 11.5f);

		m_camera->SetPosition(headPos, camera);
	}

	// Rotation
	glm::vec3 trackRot = glm::radians(tracking.rot);
	glm::vec3 rotation = rot + glm::vec3{ trackRot.x, -trackRot.y, trackRot.z };

	auto rm = glm::identity<glm::mat4>();
	rm = glm::rotate(rm, -mmath::half_pi + rotation.z, { 1.0f, 0.0f, 0.0f });
	rm = glm::rotate(rm, -rotation.x -mmath::half_pi, { 0.0f, 1.0f, 0.0f });
	rm = glm::rotate(rm, rotation.y - mmath::half_pi, { 0.0f, 0.0f, 1.0f });

	RE::NiMatrix3 cameraNiT;
	cameraNiT.entry[0][0] = rm[0][0];
	cameraNiT.entry[0][1] = rm[0][1];
	cameraNiT.entry[0][2] = rm[0][2];
	cameraNiT.entry[1][0] = rm[1][0];
	cameraNiT.entry[1][1] = rm[1][1];
	cameraNiT.entry[1][2] = rm[1][2];
	cameraNiT.entry[2][0] = rm[2][0];
	cameraNiT.entry[2][1] = rm[2][1];
	cameraNiT.entry[2][2] = rm[2][2];
	m_camera->cameraNi->world.rotate = cameraNiT;

	RE::NiMatrix3 cameraNodeT;
	cameraNodeT.entry[0][1] = cameraNiT.entry[0][0];
	cameraNodeT.entry[0][2] = cameraNiT.entry[0][1];
	cameraNodeT.entry[0][0] = cameraNiT.entry[0][2];

	cameraNodeT.entry[1][1] = cameraNiT.entry[1][0];
	cameraNodeT.entry[1][2] = cameraNiT.entry[1][1];
	cameraNodeT.entry[1][0] = cameraNiT.entry[1][2];

	cameraNodeT.entry[2][1] = cameraNiT.entry[2][0];
	cameraNodeT.entry[2][2] = cameraNiT.entry[2][1];
	cameraNodeT.entry[2][0] = cameraNiT.entry[2][2];

	camera->cameraRoot->world.rotate = cameraNodeT;
	camera->cameraRoot->local.rotate = cameraNodeT;

	// Update w2s matrix, required for correct culling
	mmath::Rotation camRot;
	camRot.SetEuler(rotation.x, rotation.y);
	m_camera->UpdateInternalWorldToScreenMatrix(camRot);
#endif
}

void Camera::Firstperson::Render(Render::D3DContext&) noexcept {}

void Camera::Firstperson::OnTogglePOV(RE::ButtonEvent*) noexcept {}

bool Camera::Firstperson::OnKeyPress(const RE::ButtonEvent*) noexcept {
	return false;
}

bool Camera::Firstperson::OnMenuOpenClose(MenuID, const RE::MenuOpenCloseEvent* const) noexcept {
	return false;
}

void Camera::Firstperson::OnCameraActionStateTransition(const RE::PlayerCharacter*, const CameraActionState,
	const CameraActionState) noexcept
{}

bool Camera::Firstperson::OnCameraStateTransition(RE::PlayerCharacter*, RE::PlayerCamera*,
	const GameState::CameraState, const GameState::CameraState) noexcept
{
	return false;
}

#ifdef DEVELOPER
void Camera::Firstperson::ToggleThirdpersonSkeleton(bool show) noexcept {
#else
void Camera::Firstperson::ToggleThirdpersonSkeleton(bool) noexcept {
#endif
#ifdef DEVELOPER
	detSwitchSkeleton->GetBase()(m_camera->currentFocusObject, show ? 0 : 1);
	tpSkeletonVisible = show;
#endif
}

void Camera::Firstperson::HidePlayerHead(bool hide) noexcept {
	if (!m_camera->currentFocusObject) return;
	if (m_camera->currentFocusObject->loadedData && m_camera->currentFocusObject->loadedData->data3D) {
		auto faceNode = m_camera->currentFocusObject->loadedData->data3D->GetObjectByName(Strings.faceNode);
		if (faceNode)
			if (hide)
				faceNode->flags.set(RE::NiAVObject::Flag::kHidden);
			else
				faceNode->flags.reset(RE::NiAVObject::Flag::kHidden);
	}
}

void Camera::Firstperson::UpdateClipableWeaponNodes(RE::PlayerCharacter* player) noexcept {
	wantClipNodes.clear();

	if (!player->loadedData || !player->loadedData->data3D) return;

	eastl::function<void(RE::NiAVObject*)> walkFun;
	walkFun = [this, &walkFun](RE::NiAVObject* obj) {
		if (!obj) return;

		auto geom = skyrim_cast<RE::BSGeometry*>(obj);
		if (geom)
			wantClipNodes.push_back(geom);

		auto no = skyrim_cast<RE::NiNode*>(obj);
		if (no)
			for (auto& child : no->children)
				if (child) walkFun(child.get());
	};

	auto accum = [&walkFun](RE::BSFixedString& nodeName, RE::NiAVObject* root) {
		auto node = root->GetObjectByName(nodeName);
		if (node) walkFun(node);
	};

	accum(Strings.SHIELD, player->loadedData->data3D.get());
	accum(Strings.WEAPON, player->loadedData->data3D.get());
}

void Camera::Firstperson::UpdateShadowOnlyNodes(RE::PlayerCharacter* player) noexcept {
	shadowsOnly.clear();
	if (!player->loadedData || !player->loadedData->data3D) return;

	// @TODO: Hook update method when root changes

	eastl::function<void(RE::NiAVObject*)> walkFun;
	walkFun = [this, &walkFun](RE::NiAVObject* obj) {
		if (!obj) return;

		auto geom = skyrim_cast<RE::BSGeometry*>(obj);
		if (geom)
			shadowsOnly.push_back(geom); // We found a mesh, insert it

		auto no = skyrim_cast<RE::NiNode*>(obj);
		if (no)
			for (auto& child : no->children)
				if (child) walkFun(child.get());
	};

	auto accum = [&walkFun](RE::BSFixedString& nodeName, RE::NiAVObject* root) {
		if (auto node = root->GetObjectByName(nodeName)) walkFun(node);
	};

	RE::BSFixedString head = "NPC Head [Head]";
	accum(Strings.faceNode, player->loadedData->data3D.get());

	RE::BSFixedString quiver = "QUIVER";
	accum(quiver, player->loadedData->data3D.get());

	auto markEquippedGeoAsShadowOnly = [this, player](RE::BGSBipedObjectForm::BipedObjectSlot slot) {
		auto obj = player->GetWornArmor(slot);
		if (!obj) return;
		for (auto& node : player->loadedData->data3D->AsNode()->children) {
			if (!node || node->name.size() == 0) continue;
			auto nodeStr = std::string(node->name);

			auto pivot = nodeStr.find_first_of('/');
			if (pivot != std::string::npos) {
				const auto formStr = Util::IntToHexStr(obj->formID);
				if (nodeStr.substr(pivot).find(formStr) != std::string::npos) {
					auto geo = skyrim_cast<RE::BSGeometry*>(node.get());
					if (geo)
						shadowsOnly.push_back(geo);

					break;
				}
			}
		}
	};

	markEquippedGeoAsShadowOnly(RE::BGSBipedObjectForm::BipedObjectSlot::kHead);
	markEquippedGeoAsShadowOnly(RE::BGSBipedObjectForm::BipedObjectSlot::kCirclet);
	markEquippedGeoAsShadowOnly(RE::BGSBipedObjectForm::BipedObjectSlot::kEars);
	markEquippedGeoAsShadowOnly(RE::BGSBipedObjectForm::BipedObjectSlot::kHair);
	markEquippedGeoAsShadowOnly(RE::BGSBipedObjectForm::BipedObjectSlot::kLongHair);
}