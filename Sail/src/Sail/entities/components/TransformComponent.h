#pragma once

#include "Component.h"
#include "../../graphics/geometry/Transform.h"

class SailGuiWindow;

class TransformComponent : public Component, public Transform {
public:
	TransformComponent(const glm::mat4& transformationMatrix)
		: Transform(transformationMatrix) { }
	TransformComponent(const glm::vec3& translation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f })
		: Transform(translation, rotation, scale) { }
	~TransformComponent() { }

	virtual void renderEditorGui(SailGuiWindow* window) override;
};
