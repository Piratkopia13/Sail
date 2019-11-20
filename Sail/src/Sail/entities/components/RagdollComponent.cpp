#include "pch.h"

#include "RagdollComponent.h"

RagdollComponent::RagdollComponent() {
	localCenterOfMass = {0.f, 0.f, 0.f};
	wireframeModel = nullptr;
}

RagdollComponent::RagdollComponent(Model* wireframe) {
	localCenterOfMass = { 0.f, 0.f, 0.f };
	wireframeModel = wireframe;
}

RagdollComponent::~RagdollComponent() {

}

void RagdollComponent::addContactPoint(glm::vec3 localOffset, glm::vec3 halfSize) {
	contactPoints.emplace_back();
	contactPoints.back().boundingBox.setHalfSize(halfSize);
	contactPoints.back().localOffSet = localOffset;
}
