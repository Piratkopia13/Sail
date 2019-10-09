#include "pch.h"
#include "AnimationSystem.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/AnimationComponent.h"
#include "Sail/entities/components/ModelComponent.h"
#include "Sail/graphics/geometry/Model.h"
#include "sail/api/VertexBuffer.h"
#include "API/DX12/DX12API.h"
#include "API/DX12/DX12VertexBuffer.h"
#include "API/DX12/resources/DescriptorHeap.h"
#include "Sail/Application.h"
#include "Sail/graphics/shader/compute/AnimationUpdateComputeShader.h"
#include "API/DX12/DX12Utils.h"
#include <glm/gtx/compatibility.hpp>


AnimationSystem::AnimationSystem() 
	: BaseComponentSystem()
	, m_interpolate(true) 
{
	// TODO: System owner should check if this is correct
	registerComponent<AnimationComponent>(true, true, true);
	registerComponent<ModelComponent>(true, true, true);

	m_updateShader = &Application::getInstance()->getResourceManager().getShaderSet<AnimationUpdateComputeShader>();
	m_dispatcher = std::unique_ptr<ComputeShaderDispatcher>(ComputeShaderDispatcher::Create());
	m_inputLayout = std::unique_ptr<InputLayout>(InputLayout::Create());
	m_inputLayout->pushVec3(InputLayout::POSITION, "POSITION", 0);
	m_inputLayout->pushVec2(InputLayout::TEXCOORD, "TEXCOORD", 0);
	m_inputLayout->pushVec3(InputLayout::NORMAL, "NORMAL", 0);
	m_inputLayout->pushVec3(InputLayout::TANGENT, "TANGENT", 0);
	m_inputLayout->pushVec3(InputLayout::BITANGENT, "BINORMAL", 0);
}

AnimationSystem::~AnimationSystem() {
}

void AnimationSystem::update(float dt) {
	updateTransforms(dt);
	updateMeshCPU();
}

void AnimationSystem::updateTransforms(const float dt) { 
	for (auto& e : entities) {
		AnimationComponent* animationC = e->getComponent<AnimationComponent>();
		addTime(animationC, dt);

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
		if (transforms1 && transforms2 && m_interpolate) {
			const float frame0Time = animationC->currentAnimation->getTimeAtFrame(frame);
			const float frame1Time = animationC->currentAnimation->getTimeAtFrame(frame2);
			// weight = time - time(0) / time(1) - time(0)
			const float w = (animationC->animationTime - frame0Time) / (frame1Time - frame0Time);
			for (unsigned int transformIndex = 0; transformIndex < transformSize; transformIndex++) {
				interpolate(animationC->transforms[transformIndex], transforms1[transformIndex], transforms2[transformIndex], w);
			}

		} else if (transforms1) {
			for (unsigned int transformIndex = 0; transformIndex < transformSize; transformIndex++) {
				animationC->transforms[transformIndex] = transforms1[transformIndex];
			}
		}
		animationC->hasUpdated = true;
	}
}

void AnimationSystem::updateMeshGPU(ID3D12GraphicsCommandList4* cmdList) {
	m_dispatcher->begin(cmdList);
	unsigned int meshIndex = 0;
	for (auto& e : entities) {
		AnimationComponent* animationC = e->getComponent<AnimationComponent>();
		ModelComponent* modelC = e->getComponent<ModelComponent>();

		if (!animationC->computeUpdate) {
			continue;
		}

		Mesh* mesh = modelC->getModel()->getMesh(0);
		const Mesh::Data* data = &mesh->getMeshData();

		// Initialize T-pose vertex buffer if it has not been done already
		if (!animationC->tposeVBuffer) {
			animationC->tposeVBuffer = std::unique_ptr<VertexBuffer>(DX12VertexBuffer::Create(*m_inputLayout, *data));
		}
		DX12VertexBuffer* tposeVBuffer = static_cast<DX12VertexBuffer*>(animationC->tposeVBuffer.get());
		// Make sure it has been initialized
		tposeVBuffer->init(cmdList);

		AnimationStack::VertConnection* connections = animationC->getAnimationStack()->getConnections();
		const unsigned int connectionSize = animationC->getAnimationStack()->getConnectionSize();

		auto* context = Application::getInstance()->getAPI<DX12API>();

		m_updateShader->getPipeline()->setStructBufferVar("CSTransforms", animationC->transforms, sizeof(glm::mat4) * animationC->transformSize, animationC->transformSize);
		m_updateShader->getPipeline()->setStructBufferVar("CSVertConnections", connections, sizeof(AnimationStack::VertConnection) * connectionSize, connectionSize);

		// Get the vertexbuffer that should contain the animated mesh
		auto& vbuffer = static_cast<DX12VertexBuffer&>(mesh->getVertexBuffer());
		// Make sure vertex buffer data has been uploaded to its default buffer
		vbuffer.init(cmdList);

		if (!animationC->hasUpdated) {
			
			DX12Utils::SetResourceTransitionBarrier(cmdList, vbuffer.getBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
			cmdList->CopyBufferRegion(vbuffer.getBuffer(), 0, vbuffer.getBuffer(-1), 0, m_inputLayout->getVertexSize() * connectionSize);
			DX12Utils::SetResourceUAVBarrier(cmdList, vbuffer.getBuffer());
			DX12Utils::SetResourceTransitionBarrier(cmdList, vbuffer.getBuffer(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			continue;
		}

		auto& cdh = context->getComputeGPUDescriptorHeap()->getCurentCPUDescriptorHandle();
		cdh.ptr += context->getComputeGPUDescriptorHeap()->getDescriptorIncrementSize() * 2; // TODO: read offset from root params

		// Create a shader resource view for the T-pose vbuffer in the correct place in the heap
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = connectionSize;
		srvDesc.Buffer.StructureByteStride = m_inputLayout->getVertexSize();
		context->getDevice()->CreateShaderResourceView(tposeVBuffer->getBuffer(), &srvDesc, cdh);
		// Transition to read access
		DX12Utils::SetResourceTransitionBarrier(cmdList, tposeVBuffer->getBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		cdh.ptr += context->getComputeGPUDescriptorHeap()->getDescriptorIncrementSize() * 8; // TODO: read offset from root params

		// Create a unordered access view for the animated vertex buffer in the correct place in the heap
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = connectionSize;
		uavDesc.Buffer.StructureByteStride = 4 * 14; // TODO: replace with sizeof(Vertex)
		context->getDevice()->CreateUnorderedAccessView(vbuffer.getBuffer(), nullptr, &uavDesc, cdh);
		// Transition to UAV access
		DX12Utils::SetResourceTransitionBarrier(cmdList, vbuffer.getBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		AnimationUpdateComputeShader::Input input;
		input.threadGroupCountX = connectionSize;
		m_dispatcher->dispatch(*m_updateShader, input, meshIndex++, cmdList);

		// Transition back to normal vertex buffer usage
		DX12Utils::SetResourceTransitionBarrier(cmdList, vbuffer.getBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		DX12Utils::SetResourceTransitionBarrier(cmdList, tposeVBuffer->getBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		/*DX12Utils::SetResourceUAVBarrier(cmdList, vbuffer.getBuffer());
		D3D12_RESOURCE_BARRIER barrierDesc = {};

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		barrierDesc.Transition.pResource = vbuffer.getBuffer();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1, &barrierDesc);*/

		vbuffer.setAsUpdated();

		animationC->hasUpdated = false;
	}
}

void AnimationSystem::updateMeshCPU() { 
	for (auto& e : entities) {
		AnimationComponent* animationC = e->getComponent<AnimationComponent>();
		ModelComponent* modelC = e->getComponent<ModelComponent>();

		if (animationC->computeUpdate) {
			continue;
		}


		Mesh* mesh = modelC->getModel()->getMesh(0);
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

			AnimationStack::VertConnection* connections = animationC->getAnimationStack()->getConnections();
			const unsigned int connectionSize = animationC->getAnimationStack()->getConnectionSize();

			// CPU UPDATE
			if (connections && animationC->transforms) {
				glm::mat4 mat;
				glm::mat4 matInv;

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
		}
	}

}

std::vector<Entity*>& AnimationSystem::getEntities() {
	return entities;
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
		if (!animationC->computeUpdate) {
			if (animationC->dataSize == 0) {
				updateMeshCPU();
			}
			ModelComponent* modelC = e->getComponent<ModelComponent>();
			Mesh* mesh = modelC->getModel()->getMesh(0);
			mesh->getVertexBuffer().update(animationC->data);
		}
	}
}

void AnimationSystem::toggleInterpolation() {
	m_interpolate = !m_interpolate;
}

const bool AnimationSystem::getInterpolation() {
	return m_interpolate;
}

void AnimationSystem::setInterpolation(const bool interpolation) {
	m_interpolate = interpolation;
}
