#include "pch.h"
#include "DX12ConstantBuffer.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"

namespace ShaderComponent {

	ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot) {
		return new DX12ConstantBuffer(initData, size, bindShader, slot);
	}

	DX12ConstantBuffer::DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot) 
		: m_register(slot)
	{
		m_context = Application::getInstance()->getAPI<DX12API>();
		auto numSwapBuffers = m_context->getNumSwapBuffers();

		m_constantBufferUploadHeap = new wComPtr<ID3D12Resource1>[numSwapBuffers];
		m_cbGPUAddress = new UINT8*[numSwapBuffers];
		//m_needsUpdate = new bool[numSwapBuffers](); // parenthesis invokes initialitaion to false

		// Create an upload heap to hold the constant buffer
		// create a resource heap, descriptor heap, and pointer to cbv for each frame
		for (UINT i = 0; i < numSwapBuffers; ++i) {
			
			// TODO: make size dynamic
			// Must be a multiple of 64KB for single-textures and constant buffers
			m_constantBufferUploadHeap[i] = DX12Utils::CreateBuffer(m_context->getDevice(), 1024 * 64, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties);
			m_constantBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

			// Map the constant buffer and keep it mapped for the duration of its lifetime
			D3D12_RANGE readRange{0, 0}; // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
			ThrowIfFailed(m_constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_cbGPUAddress[i])));

			// TODO: copy upload heap to a default heap when the data is not often changed
		}

		// Allocate cpu memory for the buffer
		// Memory leak
		//m_newData = malloc(size);
	}

	DX12ConstantBuffer::~DX12ConstantBuffer() {
		delete[] m_constantBufferUploadHeap;
		delete[] m_cbGPUAddress;
		//delete[] m_needsUpdate;
	}

	void DX12ConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int offset /*= 0U*/) {
		//TODO: change this to needsUpdate thing we have in DXR project
		auto frameIndex = m_context->getFrameIndex();
		memcpy(m_cbGPUAddress[frameIndex] + offset, newData, bufferSize);
	}

	void DX12ConstantBuffer::bind(void* cmdList) const {
		auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
		auto frameIndex = m_context->getFrameIndex();
		// TODO: convert slot to rootIndex
		UINT rootIndex = m_context->getRootIndexFromRegister("b" + std::to_string(m_register));
		dxCmdList->SetGraphicsRootConstantBufferView(rootIndex, m_constantBufferUploadHeap[frameIndex]->GetGPUVirtualAddress());
	}

}
