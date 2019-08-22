#include "pch.h"
#include "DX12Utils.h"

#include "Sail/utils/Utils.h"

const D3D12_HEAP_PROPERTIES DX12Utils::sUploadHeapProperties = {
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0,
};

const D3D12_HEAP_PROPERTIES DX12Utils::sDefaultHeapProps = {
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
};

void DX12Utils::UpdateDefaultBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* data, UINT64 byteSize, UINT64 offset, ID3D12Resource1* defaultBuffer, ID3D12Resource1** uploadBuffer) {
	// TODO: make this method useful

	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Width = byteSize;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Height = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ThrowIfFailed(device->CreateCommittedResource(
		&sUploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer)));

	// Put in barriers in order to schedule for the data to be copied
	// to the default buffer resource.
	SetResourceTransitionBarrier(cmdList, defaultBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);

	// Prepare the data to be uploaded to the GPU
	BYTE* pData;
	ThrowIfFailed((*uploadBuffer)->Map(0, NULL, reinterpret_cast<void**>(&pData)));
	memcpy(pData, data, byteSize);
	(*uploadBuffer)->Unmap(0, NULL);

	// Copy the data from the uploadBuffer to the defaultBuffer
	cmdList->CopyBufferRegion(defaultBuffer, offset, *uploadBuffer, 0, byteSize);

	// "Remove" the resource barrier
	SetResourceTransitionBarrier(cmdList, defaultBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
}

ID3D12Resource1* DX12Utils::CreateBuffer(ID3D12Device5* device, UINT64 size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps, D3D12_RESOURCE_DESC* bufDesc) {

	D3D12_RESOURCE_DESC newBufDesc = {};
	if (!bufDesc) {
		newBufDesc.Alignment = 0;
		newBufDesc.DepthOrArraySize = 1;
		newBufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		newBufDesc.Flags = flags;
		newBufDesc.Format = DXGI_FORMAT_UNKNOWN;
		newBufDesc.Height = 1;
		newBufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		newBufDesc.MipLevels = 1;
		newBufDesc.SampleDesc.Count = 1;
		newBufDesc.SampleDesc.Quality = 0;
		newBufDesc.Width = size;

		bufDesc = &newBufDesc;
	}

	ID3D12Resource1* pBuffer = nullptr;
	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer));
	return pBuffer;
}

void DX12Utils::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter) {
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}