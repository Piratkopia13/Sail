#include "pch.h"
#include "DX12ATexture.h"

DX12ATexture::DX12ATexture()
	: cpuDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2)
	, isRenderableTex(false) 
{
	// Store the cpu descriptor handle that will contain the srv for this texture
	srvHeapCDH = cpuDescHeap.getCPUDescriptorHandleForIndex(0);
	uavHeapCDH = cpuDescHeap.getCPUDescriptorHandleForIndex(1);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ATexture::getSrvCDH() const {
	return srvHeapCDH;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ATexture::getUavCDH() const {
	return uavHeapCDH;
}

void DX12ATexture::transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState) {
	if (state == newState) return;

	DX12Utils::SetResourceTransitionBarrier(cmdList, textureDefaultBuffer.Get(), state, newState);
	state = newState;
}

bool DX12ATexture::isRenderable() const {
	return isRenderableTex;
}
