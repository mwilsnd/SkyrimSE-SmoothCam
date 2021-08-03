#include "firstperson.h"

//1406a1820:39401
typedef uintptr_t(*SwitchSkeleton)(Actor*, byte);
static eastl::unique_ptr<TypedDetour<SwitchSkeleton>> detSwitchSkeleton;
static bool tpSkeletonVisible = true;
uintptr_t mSwitchSkeleton(Actor* actor, byte param_2) {
	tpSkeletonVisible = !param_2;
	return detSwitchSkeleton->GetBase()(actor, param_2);
}

Camera::Firstperson::Firstperson(Camera* baseCamera) : ICamera(baseCamera, CameraID::Firstperson) {
#ifdef DEVELOPER
	config = Config::GetCurrentConfig();

	// Game is switching player skeleton
	detSwitchSkeleton = eastl::make_unique<TypedDetour<SwitchSkeleton>>(
		39401,
		mSwitchSkeleton
	);
	detSwitchSkeleton->Attach();
#endif
}

Camera::Firstperson::~Firstperson() {}

void Camera::Firstperson::OnBegin(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* lastState) noexcept {
#ifdef DEVELOPER
	if (m_camera->currentFocusObject == player)
		ToggleThirdpersonSkeleton(true);
#endif
}

void Camera::Firstperson::OnEnd(PlayerCharacter* player, CorrectedPlayerCamera* camera, ICamera* newState) noexcept {
#ifdef DEVELOPER
	if (m_camera->currentFocusObject == player) {
		ToggleThirdpersonSkeleton(true);
		HidePlayerHead(false);
	}
#endif
}

bool Camera::Firstperson::OnPreGameUpdate(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
	return false;
}

void Camera::Firstperson::OnUpdateCamera(PlayerCharacter* player, CorrectedPlayerCamera* camera,
	BSTSmartPointer<TESCameraState>& nextState)
{
#ifdef DEVELOPER
	auto state = reinterpret_cast<CorrectedFirstPersonState*>(camera->cameraState);
	if (!state->cameraObj) return;
	if (!m_camera->currentFocusObject->loadedState || !m_camera->currentFocusObject->loadedState->node) return;

	// Make sure the thirdperson skeleton is visible
	if (!tpSkeletonVisible)
		ToggleThirdpersonSkeleton(true);

	// Make sure the player's head remains hidden
	HidePlayerHead(true);

	// Position & trackir stuff
	auto refPos = glm::vec3{
		state->cameraObj->m_worldTransform.pos.x,
		state->cameraObj->m_worldTransform.pos.y,
		state->cameraObj->m_worldTransform.pos.z
	};

	// Extract camera rotation
	auto rot = mmath::NiMatrixToEuler(m_camera->cameraNi->m_worldTransform.rot);
	// Current TrackIR data
	const auto& tracking = m_camera->trackIRData;

	auto node = m_camera->currentFocusObject->loadedState->node->GetObjectByName(&Strings.headPositionTarget.data);
	if (node) {
		glm::vec3 headPos = {
			node->m_worldTransform.pos.x,
			node->m_worldTransform.pos.y,
			node->m_worldTransform.pos.z
		};

		// And player basis vectors
		auto obj = m_camera->currentFocusObject;
		glm::vec3 f, s, u, coef;
		mmath::DecomposeToBasis(
			refPos,
			{ obj->rot.x, obj->rot.y, obj->rot.z },
			f, s, u, coef
		);

		// Position
		// @TODO: This is kind of crap, can we get some form of body IK going here?
		constexpr auto scale = 0.01f;
		glm::vec3 trackPos = tracking.pos * scale;
		headPos += (f * -trackPos.x) + (s * -trackPos.z) + (u * trackPos.y);

		m_camera->SetPosition(headPos, camera);
	}

	// Rotation
	glm::vec3 trackRot = glm::radians(tracking.rot);
	glm::vec3 rotation = rot + glm::vec3{ trackRot.x, -trackRot.y, trackRot.z };

	auto rm = glm::identity<glm::mat4>();
	rm = glm::rotate(rm, -mmath::half_pi + rotation.z, { 1.0f, 0.0f, 0.0f });
	rm = glm::rotate(rm, -rotation.x -mmath::half_pi, { 0.0f, 1.0f, 0.0f });
	rm = glm::rotate(rm, rotation.y - mmath::half_pi, { 0.0f, 0.0f, 1.0f });

	NiMatrix33 cameraNiT;
	cameraNiT.data[0][0] = rm[0][0];
	cameraNiT.data[0][1] = rm[0][1];
	cameraNiT.data[0][2] = rm[0][2];
	cameraNiT.data[1][0] = rm[1][0];
	cameraNiT.data[1][1] = rm[1][1];
	cameraNiT.data[1][2] = rm[1][2];
	cameraNiT.data[2][0] = rm[2][0];
	cameraNiT.data[2][1] = rm[2][1];
	cameraNiT.data[2][2] = rm[2][2];
	m_camera->cameraNi->m_worldTransform.rot = cameraNiT;

	NiMatrix33 cameraNodeT;
	cameraNodeT.arr[1] = cameraNiT.arr[0];
	cameraNodeT.arr[2] = cameraNiT.arr[1];
	cameraNodeT.arr[0] = cameraNiT.arr[2];
	cameraNodeT.arr[4] = cameraNiT.arr[3];
	cameraNodeT.arr[5] = cameraNiT.arr[4];
	cameraNodeT.arr[3] = cameraNiT.arr[5];
	cameraNodeT.arr[7] = cameraNiT.arr[6];
	cameraNodeT.arr[8] = cameraNiT.arr[7];
	cameraNodeT.arr[6] = cameraNiT.arr[8];
	camera->cameraNode->m_worldTransform.rot = cameraNodeT;
	camera->cameraNode->m_localTransform.rot = cameraNodeT;

	// Update w2s matrix, required for correct culling
	mmath::Rotation camRot;
	camRot.SetEuler(rotation.x, rotation.y);
	m_camera->UpdateInternalWorldToScreenMatrix(camRot, camera);
#endif
}

void Camera::Firstperson::Render(Render::D3DContext& ctx) noexcept {}

void Camera::Firstperson::OnTogglePOV(const ButtonEvent* ev) noexcept {}

bool Camera::Firstperson::OnKeyPress(const ButtonEvent* ev) noexcept {
	return false;
}

bool Camera::Firstperson::OnMenuOpenClose(MenuID id, const MenuOpenCloseEvent* const ev) noexcept {
	return false;
}

void Camera::Firstperson::OnCameraActionStateTransition(const PlayerCharacter* player, const CameraActionState newState,
	const CameraActionState oldState) noexcept
{}

void Camera::Firstperson::OnCameraStateTransition(const PlayerCharacter* player, const CorrectedPlayerCamera* camera,
	const GameState::CameraState newState, const GameState::CameraState oldState) noexcept
{}

void Camera::Firstperson::ToggleThirdpersonSkeleton(bool show) noexcept {
	detSwitchSkeleton->GetBase()(m_camera->currentFocusObject, show ? 0 : 1);
	tpSkeletonVisible = show;
}

void Camera::Firstperson::HidePlayerHead(bool hide) noexcept {
	if (!m_camera->currentFocusObject) return;
	if (m_camera->currentFocusObject->loadedState && m_camera->currentFocusObject->loadedState->node) {
		auto faceNode = m_camera->currentFocusObject->loadedState->node->GetObjectByName(&Strings.faceNode.data);
		if (faceNode)
			reinterpret_cast<eastl::bitset<32>*>(&faceNode->m_flags)->set(0, hide);
	}
}