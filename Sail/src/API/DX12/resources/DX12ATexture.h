#pragma once

#include "../DX12API.h"
#include "DescriptorHeap.h"
#include "../DX12Utils.h"

class DX12ATexture {
public:
	DX12ATexture() 
		: m_cpuDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1)
		, isRenderableTex(false)
	{
		// Store the cpu descriptor handle that will contain the srv for this texture
		heapCDH = m_cpuDescHeap.getCPUDescriptorHandleForIndex(0);
	}
	virtual ~DX12ATexture() {};

	D3D12_CPU_DESCRIPTOR_HANDLE getCDH() const {
		return heapCDH;
	}

	void transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState) {
		if (state == newState) return;

		DX12Utils::SetResourceTransitionBarrier(cmdList, textureDefaultBuffer.Get(), state, newState);
		state = newState;
	}

	bool isRenderable() const {
		return isRenderableTex;
	}

protected:
	wComPtr<ID3D12Resource1> textureDefaultBuffer;
	DescriptorHeap m_cpuDescHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE heapCDH;
	D3D12_RESOURCE_STATES state;
	bool isRenderableTex;
};