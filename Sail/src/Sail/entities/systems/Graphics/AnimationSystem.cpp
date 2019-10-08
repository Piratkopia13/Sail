#include "pch.h"
#include "AnimationSystem.h"
#include "..//..//Entity.h"
#include "..//..//components/AnimationComponent.h"
#include "..//..//components/ModelComponent.h"
#include "Sail/graphics/geometry/Model.h"
#include "sail/api/VertexBuffer.h"
#include "API/DX12/DX12VertexBuffer.h"
#include <glm/gtx/compatibility.hpp>


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
		addTime(animationC, dt);

		Mesh * mesh = modelC->getModel()->getMesh(0);
		const Mesh::Data* data = &mesh->getMeshData();
		if (data->numVertices > 0) {
			if (animationC->data.numVertices != data->numVertices) {
				animationC->data.deepCopy(*data);
			}
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
			if (transformSize != animationC->transformSize) {
				Memory::SafeDeleteArr(animationC->transforms);
				animationC->transforms = SAIL_NEW glm::mat4[animationC->currentAnimation->getAnimationTransformSize(unsigned int(0))];

				#if defined(_DEBUG) && defined(SAIL_VERBOSELOGGING)
								Logger::Log("AnimationSystem: Rebuilt transformarray");
				#endif
			}
			

			const glm::mat4* transforms1 = animationC->currentAnimation->getAnimationTransform(frame);
			const glm::mat4* transforms2 = frame2 > frame ? animationC->currentAnimation->getAnimationTransform(frame2) : nullptr;
		
			
			//INTERPOLATE
			if (transforms1 && transforms2) {
				const float frame0Time = animationC->currentAnimation->getTimeAtFrame(frame);
				const float frame1Time = animationC->currentAnimation->getTimeAtFrame(frame2);
				// weight = time - time(0) / time(1) - time(0)
				const float w = (animationC->animationTime - frame0Time)- (frame1Time - frame0Time);
				for (unsigned int transformIndex = 0; transformIndex < transformSize; transformIndex++) {
					interpolate(animationC->transforms[transformIndex], transforms1[transformIndex], transforms2[transformIndex], w);
				}

			}
			else if (transforms1) {
				for (unsigned int transformIndex = 0; transformIndex < transformSize; transformIndex++) {
					animationC->transforms[transformIndex] = transforms1[transformIndex];
				}
			}
			AnimationStack::VertConnection* connections = animationC->getAnimationStack()->getConnections();
			const unsigned int connectionSize = animationC->getAnimationStack()->getConnectionSize();





			// CPU UPDATE
			if (connections && transforms1) {
				glm::mat mat = glm::identity<glm::mat4>();
				glm::mat matInv = glm::identity<glm::mat4>();

				for (unsigned int connectionIndex = 0; connectionIndex < connectionSize; connectionIndex++) {
					unsigned int count = connections[connectionIndex].count;
					mat = glm::zero<glm::mat4>();
					matInv = glm::zero<glm::mat4>();

					float weightTotal = 0.0f;
					for (unsigned int countIndex = 0; countIndex < count; countIndex++) {
						mat += animationC->transforms[connections[connectionIndex].transform[countIndex]] * connections[connectionIndex].weight[countIndex];
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

void AnimationSystem::addTime(AnimationComponent* e, const float time) {
	e->animationTime += time * e->animationSpeed;
	if (e->animationTime >= e->currentAnimation->getMaxAnimationTime()) {
		e->animationTime -= (int(e->animationTime / e->currentAnimation->getMaxAnimationTime()) * e->currentAnimation->getMaxAnimationTime());
	}
}

void AnimationSystem::interpolate(glm::mat4& res, const glm::mat4& mat1, const glm::mat4& mat2, const float w) {
	glm::vec3 pos[2], scale[2];
	glm::quat rot[2];

	glm::decompose(mat1, scale[0], rot[0], pos[0], glm::vec3(), glm::vec4()); 
	glm::decompose(mat2, scale[1], rot[1], pos[1], glm::vec3(), glm::vec4()); 

	glm::vec3 resScale = glm::lerp(scale[0], scale[1], w);
	glm::vec3 resPos = glm::lerp(pos[0], pos[1], w);
	glm::quat resRot = glm::slerp(rot[0], rot[1], w);

	res = glm::translate(glm::identity<glm::mat4>(), resPos);
	res = glm::scale(res, resScale);
	res = res * glm::toMat4(resRot);
}

void AnimationSystem::updatePerFrame(float dt) {
	for (auto& e : entities) {
		AnimationComponent* animationC = e->getComponent<AnimationComponent>();
		ModelComponent* modelC = e->getComponent<ModelComponent>();
		Mesh* mesh = modelC->getModel()->getMesh(0);
		mesh->getVertexBuffer().update(animationC->data);
	}
}
