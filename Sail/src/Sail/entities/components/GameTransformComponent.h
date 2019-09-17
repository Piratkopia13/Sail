#pragma once

#include "Component.h"
#include "../../graphics/geometry/transform/GameTransform.h"


// The transform component used for all the CPU updates
class GameTransformComponent : public Component<GameTransformComponent>, public GameTransform {
public:
	explicit GameTransformComponent(GameTransformComponent* parent)
		: GameTransform(parent) {}

	GameTransformComponent(const glm::vec3& translation, GameTransformComponent* parent)
		: GameTransform(translation, parent) {}

	GameTransformComponent(
		const glm::vec3& translation = { 0.0f, 0.0f, 0.0f }, 
		const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, 
		const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, 
		GameTransformComponent* parent = nullptr)
		: GameTransform(translation, rotation, scale, parent) {}

	//TransformComponent:

	virtual ~GameTransformComponent() {}
private:

};