#pragma once

#include <d3d12.h>

class DX12Utils {
public:

	static const D3D12_HEAP_PROPERTIES sUploadHeapProperties;
	static const D3D12_HEAP_PROPERTIES sDefaultHeapProps;

	static void UpdateDefaultBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* data, UINT64 byteSize, UINT64 offset, ID3D12Resource1* defaultBuffer, ID3D12Resource1** uploadBuffer);
	static ID3D12Resource1* CreateBuffer(ID3D12Device5* device, UINT64 size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps, D3D12_RESOURCE_DESC* bufDesc = nullptr);
	static void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

};