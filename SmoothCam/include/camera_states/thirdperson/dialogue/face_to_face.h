#pragma once
#ifdef DEVELOPER
#include "camera_states/thirdperson/thirdperson_dialogue.h"
#ifdef DEBUG
#include "render/line_drawer.h"
#endif

namespace Camera {
	namespace State {
		class FaceToFaceDialogue : public IThirdPersonDialogue {
			public:
				explicit FaceToFaceDialogue(ThirdpersonDialogueState* parentState) noexcept;
				virtual ~FaceToFaceDialogue() noexcept;

			public:
				virtual void OnStart(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual void OnEnd(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual void Update(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual void ExitPoll(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;
				virtual bool CanExit(RE::PlayerCharacter* player, RE::Actor* cameraRef,
					RE::PlayerCamera* playerCamera) noexcept override;

#ifdef DEBUG
				void Draw(Render::D3DContext& ctx) noexcept;
				void DrawBounds(RE::NiAVObject* obj, const glm::vec4&& col) noexcept;
				void DrawBounds(const glm::vec3& loc, const glm::vec4&& col) noexcept;

				eastl::unique_ptr<Render::LineDrawer> segmentDrawer;
				Render::LineList segments;
				bool drawOverlay = false;
				bool allowDraw = false;

				struct VSMatricesCBuffer {
					glm::mat4 matProjView = glm::identity<glm::mat4>();
					glm::vec4 tint = { 1.0f, 1.0f, 1.0f, 1.0f };
					float curTime = 0.0f;
					float pad[3] = { 0.0f, 0.0f, 0.0f };
				};
				static_assert(sizeof(VSMatricesCBuffer) % 16 == 0);

				VSMatricesCBuffer cbufPerFrameStaging = {};
				eastl::shared_ptr<Render::CBuffer> cbufPerFrame;
#endif

			private:
				glm::vec3 GetActorGoalPos(RE::TESObjectREFR* actor, const glm::mat4&& rotation, bool flipShoulder) noexcept;
				glm::vec3 GetActorFocalPos(RE::TESObjectREFR* actor) noexcept;
				RE::TESObjectREFR* SelectFocalActor(RE::PlayerCharacter* player, RE::TESObjectREFR* actor) noexcept;
				bool IsDelayTimerExpired() noexcept;

			private:
				bool runningExit = false;
				bool exitDone = false;

				// Is the camer currently looking at the player
				bool isPlayerFocus = false;
				// Timer set when entering/exiting dialogue or when actor focus flips
				float zoomStartTime = 0.0f;
				// Move duration for actor flipping
				float curMoveDuration = 1.0f;
				// Initial actor focus delay on entering dialogue
				float actorDelayStartTime = 0.0f;
				// Wait duration before the first rotation
				static constexpr float actorWaitDelay = 0.5f;
				// Initial buffer duration when starting dialogue
				static constexpr float actorInitialWaitDelay = 1.25f;
				// True if we are awaiting the inital buffer delay
				bool awaitBufferDelay = false;
				// Last actor we were focused on
				RE::TESObjectREFR* lastFocalActor = nullptr;
				// Last set position
				glm::vec3 lastPosition{};
				// Camera position at dialogue entry
				glm::vec3 startPosition{};

				mmath::Rotation lastRotation;
				mmath::Rotation lookRotation;

				struct {
					mutable RE::BSFixedString head = "NPC Head [Head]";
				} strings;

				enum class PositionMode : uint8_t {
					// Swap along the shoulder side with the shortest distance between both actors
					SwapShorestPath,

					// Swap along the shoulder side with the furthest distance between both actors
					// Selected if the shortest path is blocked by something
					SwapFurthestPath,
					
					// Ex: Two actors sitting at a bar/counter and talking side-by side while looking forward, angle between them is ~180
					// Shoulder swap in this mode instead flips from camera behind the actors to camera in front
					// @Note: if shoulder swapped and in-front is blocked, or vice versa, the camera will auto swap to the free path
					SwapSitting180,
				};
		};
	}
}
#endif