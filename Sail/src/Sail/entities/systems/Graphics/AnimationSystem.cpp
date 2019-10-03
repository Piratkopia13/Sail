#include "pch.h"
#include "AnimationSystem.h"
#include "..//..//Entity.h"
#include "..//..//components/AnimationComponent.h"
#include "..//..//components/ModelComponent.h"
#include "Sail/graphics/geometry/Model.h"
#include "sail/api/VertexBuffer.h"
#include "API/DX12/DX12VertexBuffer.h"


AnimationSystem::AnimationSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<AnimationComponent>(true, true, true);
	registerComponent<ModelComponent>(true, true, true);
}

AnimationSystem::~AnimationSystem() {
}

void AnimationSystem::update(float dt) {
	for (auto& e : entities) {
		AnimationComponent* animationC = e->getComponent<AnimationComponent>();
		ModelComponent* modelC = e->getComponent<ModelComponent>();
		animationC->animationTime += dt;
		if (animationC->animationTime >= animationC->currentAnimation->getMaxAnimationTime())
			animationC->animationTime -= animationC->currentAnimation->getMaxAnimationTime();

		Mesh * mesh = modelC->getModel()->getMesh(0);
		const Mesh::Data* data = &mesh->getMeshData();
		if (data->numVertices > 0) {
			if (animationC->data.numVertices != data->numVertices)
				animationC->data.deepCopy(*data);
			if (animationC->dataSize != data->numVertices) {
				animationC->resizeData(data->numVertices);
			}
			const Mesh::vec3* pos = data->positions;
			const Mesh::vec3* norm = data->normals;
			const Mesh::vec3* tangent = data->tangents;
			const Mesh::vec3* bitangent = data->bitangents;
			const Mesh::vec2* uv = data->texCoords;

			const unsigned int frame = animationC->currentAnimation->getFrameAtTime(animationC->animationTime, Animation::BEHIND);
			const unsigned int frame2 = animationC->currentAnimation->getFrameAtTime(animationC->animationTime, Animation::INFRONT);
			const unsigned int transformSize = animationC->currentAnimation->getAnimationTransformSize(frame);
			const unsigned int transformSize2 = animationC->currentAnimation->getAnimationTransformSize(frame2);

			const glm::mat4* transforms = animationC->currentAnimation->getAnimationTransform(frame);
			const glm::mat4* transforms2 = animationC->currentAnimation->getAnimationTransform(frame2);

			AnimationStack::VertConnection* connections = animationC->getAnimationStack()->getConnections();
			const unsigned int connectionSize = animationC->getAnimationStack()->getConnectionSize();

			if (connections && transforms) {
				glm::mat mat = glm::identity<glm::mat4>();
				glm::mat matInv = glm::identity<glm::mat4>();

				for (unsigned int connectionIndex = 0; connectionIndex < connectionSize; connectionIndex++) {
					unsigned int count = connections[connectionIndex].count;
					mat = glm::zero<glm::mat4>();
					matInv = glm::zero<glm::mat4>();

					float weightTotal = 0.0f;
					for (unsigned int countIndex = 0; countIndex < count; countIndex++) {
						mat += transforms[connections[connectionIndex].transform[countIndex]] * connections[connectionIndex].weight[countIndex];
						weightTotal += connections[connectionIndex].weight[countIndex];
					}
					matInv = glm::inverseTranspose(mat);



					animationC->data.positions[connectionIndex].vec = glm::vec3(mat * glm::vec4(pos[connectionIndex].vec, 1));
					animationC->data.normals[connectionIndex].vec = glm::vec3(matInv * glm::vec4(norm[connectionIndex].vec, 0));
					animationC->data.tangents[connectionIndex].vec = glm::vec3(matInv * glm::vec4(tangent[connectionIndex].vec, 0));
					animationC->data.bitangents[connectionIndex].vec = glm::vec3(matInv * glm::vec4(bitangent[connectionIndex].vec, 0));

					animationC->data.texCoords[connectionIndex].vec = uv[connectionIndex].vec;
				}
			}
			mesh->getVertexBuffer().update(animationC->data);
		}
	}
}
