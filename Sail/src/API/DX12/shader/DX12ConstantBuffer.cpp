#include "pch.h"
#include "DX12ConstantBuffer.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"

namespace ShaderComponent {

	ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot) {
		return SAIL_NEW DX12ConstantBuffer(initData, size, bindShader, slot);
	}

	DX12ConstantBuffer::DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot) 
		: m_register(slot)
		, m_resourceHeapMeshIndex(0)
	{
		m_context = Application::getInstance()->getAPI<DX12API>();
		auto numSwapBuffers = m_context->getNumSwapBuffers();

		// Store offset size for each mesh
		m_byteAlignedSize = (size + 255) & ~255;
		// Size must be a multiple of 64KB for single-textures and constant buffers
		m_resourceHeapSize = (unsigned int)((glm::floor(size / (1024.0 * 64.0)) + 1) * (1024.0 * 64.0));

		m_constantBufferUploadHeap.resize(numSwapBuffers);
		m_cbGPUAddress.resize(numSwapBuffers);
		//m_needsUpdate = new bool[numSwapBuffers](); // Parenthesis invokes initialitaion to false

		createBuffers();
		for (UINT i = 0; i < numSwapBuffers; i++) {
			// Place initData in the buffer
			memcpy(m_cbGPUAddress[i], initData, size);
		}
	}

	DX12ConstantBuffer::~DX12ConstantBuffer() {
		//delete[] m_needsUpdate;
	}

	void DX12ConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int offset /*= 0U*/) {
		// This method needs to be run every frame to make sure the buffer for all framebuffers are kept updated
		auto frameIndex = m_context->getFrameIndex();
		memcpy(m_cbGPUAddress[frameIndex] + m_byteAlignedSize * m_resourceHeapMeshIndex + offset, newData, bufferSize);
	}

	void DX12ConstantBuffer::bind(void* cmdList) const {
		auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
		auto frameIndex = m_context->getFrameIndex();
		
		UINT rootIndex = m_context->getRootIndexFromRegister("b" + std::to_string(m_register));
		dxCmdList->SetGraphicsRootConstantBufferView(rootIndex, m_constantBufferUploadHeap[frameIndex]->GetGPUVirtualAddress() + m_byteAlignedSize * m_resourceHeapMeshIndex);
	}

	void DX12ConstantBuffer::setResourceHeapMeshIndex(unsigned int index) {
		m_resourceHeapMeshIndex = index;
		auto numSwapBuffers = m_context->getNumSwapBuffers();
		auto frameIndex = m_context->getFrameIndex();
		// Expand resource heap if index is out of range
		if ((index + 1) * m_byteAlignedSize >= m_resourceHeapSize) {
			unsigned int oldSize = m_resourceHeapSize;
			m_resourceHeapSize += 1024 * 64;

			// Copy gpu memory to ram before recreating buffers
			void* data = malloc(oldSize);
			memcpy(data, m_cbGPUAddress[frameIndex], oldSize);

			createBuffers();
			for (UINT i = 0; i < numSwapBuffers; i++) {
				// Place the original data in the buffer
				memcpy(m_cbGPUAddress[i], data, oldSize);
			}
		}
	}

	void DX12ConstantBuffer::createBuffers() {
		auto numSwapBuffers = m_context->getNumSwapBuffers();

		// Create an upload heap to hold the constant buffer
		// create a resource heap, and pointer to cbv for each frame
		for (UINT i = 0; i < numSwapBuffers; i++) {

			m_constantBufferUploadHeap[i].Attach(DX12Utils::CreateBuffer(m_context->getDevice(), m_resourceHeapSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties));
			m_constantBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

			// Map the constant buffer and keep it mapped for the duration of its lifetime
			D3D12_RANGE readRange{ 0, 0 }; // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
			ThrowIfFailed(m_constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_cbGPUAddress[i])));

			// TODO: copy upload heap to a default heap when the data is not often changed
		}
	}

}
