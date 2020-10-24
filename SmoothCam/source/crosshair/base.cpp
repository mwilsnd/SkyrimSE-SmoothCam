#include "crosshair/base.h"

void Crosshair::Base::SetPosition(const glm::vec3& pos) {
	position = pos;
	dirty = true;
}

void Crosshair::Base::SetRotation(const glm::vec3& rot) {
	rotation = rot;
	dirty = true;
}

void Crosshair::Base::SetScale(const glm::vec3& s) {
	scale = s;
	dirty = true;
}

void Crosshair::Base::UpdateTransform() {
	if (!dirty) return;
	transform = glm::identity<glm::mat4>();
	transform = glm::translate(transform, Render::ToRenderScale(position));
	transform = glm::rotate(transform, rotation.y, { 0.0f, 0.0f, 1.0f });
	transform = glm::rotate(transform, rotation.x, { 1.0f, 0.0f, 0.0f });
	transform = glm::rotate(transform, rotation.z, { 0.0f, 1.0f, 0.0f });
	transform = glm::scale(transform, scale);
	dirty = false;
}

glm::vec3 Crosshair::Base::GetPosition() const noexcept {
	return position;
}

glm::vec3 Crosshair::Base::GetRotation() const noexcept {
	return rotation;
}

glm::vec3 Crosshair::Base::GetScale() const noexcept {
	return scale;
}

glm::mat4 Crosshair::Base::GetTransform() const noexcept {
	return transform;
}