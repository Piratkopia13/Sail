#include "pch.h"
#include "DX12ATexture.h"

DX12ATexture::DX12ATexture()
	: cpuDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1)
	, isRenderableTex(false) 
{
	// Store the cpu descriptor handle that will contain the srv for this texture
	heapCDH = cpuDescHeap.getCPUDescriptorHandleForIndex(0);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ATexture::getCDH() const {
	return heapCDH;
}

void DX12ATexture::transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState) {
	if (state == newState) return;

	DX12Utils::SetResourceTransitionBarrier(cmdList, textureDefaultBuffer.Get(), state, newState);
	state = newState;
}

bool DX12ATexture::isRenderable() const {
	return isRenderableTex;
}
