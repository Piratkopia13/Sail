#include "pch.h"
#include "DX12IndexBuffer.h"
#include "DX12Utils.h"
#include "Sail/Application.h"

IndexBuffer* IndexBuffer::Create(Mesh::Data& modelData) {
	return SAIL_NEW DX12IndexBuffer(modelData);
}

// All index buffers are currently handles as static and cannot be changed after upload
// TODO: add support for dynamic index buffers (that may change)
DX12IndexBuffer::DX12IndexBuffer(Mesh::Data& modelData) 
	: IndexBuffer(modelData)
	, m_hasBeenInitialized(false)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<DX12API>();

	unsigned long* indices = getIndexData(modelData);
	auto numSwapBuffers = m_context->getNumSwapBuffers();

	m_byteSize = getIndexDataSize();

	// Create buffers
	{
		m_uploadIndexBuffers.Attach(DX12Utils::CreateBuffer(m_context->getDevice(), m_byteSize,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			DX12Utils::sUploadHeapProperties));
		m_uploadIndexBuffers->SetName(L"Index buffer upload heap");

		// Place indices in the buffer
		void* pData;
		D3D12_RANGE readRange{ 0, 0 };
		ThrowIfFailed(m_uploadIndexBuffers->Map(0, &readRange, &pData));
		memcpy(pData, indices, m_byteSize);
		m_uploadIndexBuffers->Unmap(0, nullptr);

		// Create the default buffers that the data will be copied to during init()
		m_defaultIndexBuffers.Attach(DX12Utils::CreateBuffer(m_context->getDevice(), m_byteSize,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_COPY_DEST, DX12Utils::sDefaultHeapProps));
		m_defaultIndexBuffers->SetName(L"Index buffer default heap");
	}

	// Delete indices from cpu memory
	Memory::SafeDeleteArr(indices);
}

DX12IndexBuffer::~DX12IndexBuffer() {
}

void DX12IndexBuffer::bind(void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	// Make sure it has been initialized
	init(dxCmdList);

	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = m_defaultIndexBuffers->GetGPUVirtualAddress();
	ibView.SizeInBytes = static_cast<UINT>(getIndexDataSize());
	ibView.Format = DXGI_FORMAT_R32_UINT;
	
	dxCmdList->IASetIndexBuffer(&ibView);
}

ID3D12Resource* DX12IndexBuffer::getResource() const {
	assert(m_hasBeenInitialized && "Index buffer has to be initialized before call to getResource()");
	return m_defaultIndexBuffers.Get();
}

bool DX12IndexBuffer::init(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// This method is called at least once every frame that this index buffer is used

	if (m_hasBeenInitialized) {
		// Release the upload heap as soon as the data has been uploaded to the GPU, but make sure it doesn't happen on the same frame as the upload
		if (m_uploadIndexBuffers && m_initFrameCount != m_context->getFrameCount() && m_queueUsedForUpload->getCompletedFenceValue() > m_initFenceVal) {
			m_uploadIndexBuffers.ReleaseAndGetAddressOf();
		}
		return true;
	}

	// Copy the data from the uploadBuffer to the defaultBuffer
	cmdList->CopyBufferRegion(m_defaultIndexBuffers.Get(), 0, m_uploadIndexBuffers.Get(), 0, m_byteSize);
	// Transition to usage state
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_defaultIndexBuffers.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	auto type = cmdList->GetType();
	m_queueUsedForUpload = (type == D3D12_COMMAND_LIST_TYPE_DIRECT) ? m_context->getDirectQueue() : nullptr; // TODO: add support for other command list types
	// Schedule a signal to be called directly after this command list has executed
	// This allows us to know when the data has been uploaded to the GPU
	m_queueUsedForUpload->scheduleSignal([this](UINT64 signaledValue) {
		m_initFenceVal = signaledValue;
	});
	m_initFrameCount = m_context->getFrameCount();

	m_hasBeenInitialized = true;

	return false;
}