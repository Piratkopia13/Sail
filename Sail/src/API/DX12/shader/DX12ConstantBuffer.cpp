#include "pch.h"
#include "DX12ConstantBuffer.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"

namespace ShaderComponent {

	ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader) {
		return SAIL_NEW DX12ConstantBuffer(initData, size, bindShader, slot, inComputeShader);
	}

	DX12ConstantBuffer::DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader)
		: ConstantBuffer(slot)
		, m_inComputeShader(inComputeShader) {
		SAIL_PROFILE_API_SPECIFIC_FUNCTION();

		m_context = Application::getInstance()->getAPI<DX12API>();
		auto numSwapBuffers = m_context->getNumSwapBuffers();

		// Store offset size for each mesh
		m_byteAlignedSize = (size + 255) & ~255;

		m_resourceHeapSizes.resize(numSwapBuffers);
		m_constantBufferUploadHeap.resize(numSwapBuffers);
		m_cbGPUAddress.resize(numSwapBuffers);
		//m_needsUpdate = new bool[numSwapBuffers](); // Parenthesis invokes initialitaion to false

		for (UINT i = 0; i < numSwapBuffers; i++) {
			// Size must be a multiple of 64KB for single-textures and constant buffers
			m_resourceHeapSizes[i] = (unsigned int)((glm::floor(size / (1024.0 * 64.0)) + 1) * (1024.0 * 64.0));
			// Create the buffer
			createBuffer(i);
			// Place initData in the buffer
			memcpy(m_cbGPUAddress[i], initData, size);
		}
	}

	DX12ConstantBuffer::~DX12ConstantBuffer() {
		//delete[] m_needsUpdate;
	}

	void DX12ConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int offset) {
		// This method needs to be run every frame to make sure the buffer for all framebuffers are kept updated
		auto frameIndex = m_context->getSwapIndex();
		memcpy(m_cbGPUAddress[frameIndex] + offset, newData, bufferSize);
	}

	void DX12ConstantBuffer::bind(void* cmdList) const {
		SAIL_PROFILE_API_SPECIFIC_FUNCTION();

		auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
		auto frameIndex = m_context->getSwapIndex();

		UINT rootIndex = m_context->getRootSignEntryFromRegister("b" + std::to_string(slot)).rootSigIndex;

		if (m_inComputeShader) {
			dxCmdList->SetComputeRootConstantBufferView(rootIndex, m_constantBufferUploadHeap[frameIndex]->GetGPUVirtualAddress());
		} else {
			dxCmdList->SetGraphicsRootConstantBufferView(rootIndex, m_constantBufferUploadHeap[frameIndex]->GetGPUVirtualAddress());
		}
	}

	ID3D12Resource* DX12ConstantBuffer::getBuffer() const {
		return m_constantBufferUploadHeap[m_context->getSwapIndex()].Get();
	}

	void DX12ConstantBuffer::createBuffer(unsigned int swapIndex) {
		SAIL_PROFILE_API_SPECIFIC_FUNCTION();

		// Create an upload heap to hold the constant buffer
		m_constantBufferUploadHeap[swapIndex].Attach(DX12Utils::CreateBuffer(m_context->getDevice(), m_resourceHeapSizes[swapIndex], D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties));
		m_constantBufferUploadHeap[swapIndex]->SetName(L"Constant Buffer Upload Resource Heap");

		// Map the constant buffer and keep it mapped for the duration of its lifetime
		D3D12_RANGE readRange{ 0, 0 }; // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
		ThrowIfFailed(m_constantBufferUploadHeap[swapIndex]->Map(0, &readRange, reinterpret_cast<void**>(&m_cbGPUAddress[swapIndex])));

		// TODO: copy upload heap to a default heap when the data is not often changed
	}
}