#include "pch.h"
#include "AnimationSystem.h"
#include "..//..//Entity.h"
#include "..//..//components/AnimationComponent.h"
#include "..//..//components/ModelComponent.h"
#include "Sail/graphics/geometry/Model.h"
#include "sail/api/VertexBuffer.h"
#include "API/DX12/DX12VertexBuffer.h"


AnimationSystem::AnimationSystem() : BaseComponentSystem() {
	m_requiredComponentTypes.push_back(AnimationComponent::ID);
	m_requiredComponentTypes.push_back(ModelComponent::ID);
}

AnimationSystem::~AnimationSystem() {
}

void AnimationSystem::update(float dt) {
	for (auto& e : m_entities) {
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
			const unsigned int transformSize = animationC->currentAnimation->getAnimationTransformSize(frame);
			const glm::mat4* transforms = animationC->currentAnimation->getAnimationTransform(frame);
			Logger::Log(std::to_string(frame));


			AnimationStack::VertConnection* connections = animationC->getAnimationStack()->getConnections();
			const unsigned int connectionSize = animationC->getAnimationStack()->getConnectionSize();


			if (connections && transforms) {
				glm::mat mat = glm::identity<glm::mat4>();
				for (unsigned int connectionIndex = 0; connectionIndex < connectionSize; connectionIndex++) {
					unsigned int count = connections[connectionIndex].count;
					mat = glm::identity<glm::mat4>();
					for (unsigned int countIndex = 0; countIndex < count; countIndex++) {
						mat += transforms[connections[connectionIndex].transform[countIndex]] * connections[connectionIndex].weight[countIndex];
					}
					animationC->data.positions[connectionIndex].vec = glm::vec3( mat * glm::vec4(pos[connectionIndex].vec,1));
					animationC->data.normals[connectionIndex].vec = glm::vec3( mat * glm::vec4(norm[connectionIndex].vec,1));
					//animationC->data.positions[connectionIndex].vec = pos[connectionIndex].vec;
					//animationC->data.normals[connectionIndex].vec = norm[connectionIndex].vec;
					animationC->data.tangents[connectionIndex].vec = glm::vec3( mat * glm::vec4(tangent[connectionIndex].vec,1));
					animationC->data.bitangents[connectionIndex].vec = glm::vec3( mat * glm::vec4(bitangent[connectionIndex].vec,1));
					animationC->data.texCoords[connectionIndex].vec = uv[connectionIndex].vec;
					#ifdef _DEBUG
						if (connectionIndex == 0) {
						const int x = animationC->data.positions[connectionIndex].vec.x;
						const int y = animationC->data.positions[connectionIndex].vec.y;
						const int z = animationC->data.positions[connectionIndex].vec.z;
						Logger::Log("   (" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z)+")");




					}
					#endif

				}


			}
			
			mesh->getVertexBuffer().update(animationC->data);


		}

		


		//TransformComponent* transform = e->getComponent<TransformComponent>();
		//PhysicsComponent* physics = e->getComponent<PhysicsComponent>();
		//
		//transform->rotate(physics->rotation * dt);
		//transform->translate(physics->velocity * dt + physics->acceleration * (dt * dt * 0.5f));
		//
		//physics->velocity += physics->acceleration * dt;
	}
}
