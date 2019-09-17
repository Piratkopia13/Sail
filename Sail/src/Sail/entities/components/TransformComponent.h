//#pragma once
//
//#include "Component.h"
//#include "../../graphics/geometry/Transform.h"
//
//class TransformComponent : public Component<TransformComponent>, public Transform {
//public:
//	explicit TransformComponent(TransformComponent* parent)
//		: Transform(parent) {}
//
//	TransformComponent(const glm::vec3& translation, TransformComponent* parent) 
//		: Transform(translation, parent) {}
//
//	TransformComponent(const glm::vec3& translation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, TransformComponent* parent = nullptr)
//		: Transform(translation, rotation, scale, parent) {}
//
//	//TransformComponent:
//
//	~TransformComponent() { }
//
//	void setTransformFromGameObject(TransformComponent* gameObjectTransform) {
//
//	}
//
//private:
//
//};