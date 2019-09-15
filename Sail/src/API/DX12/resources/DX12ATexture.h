#pragma once

#include "../DX12API.h"
#include "DescriptorHeap.h"
#include "../DX12Utils.h"

class DX12ATexture {
public:
	DX12ATexture();
	~DX12ATexture() {};

	D3D12_CPU_DESCRIPTOR_HANDLE getCDH() const;
	void transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState);
	bool isRenderable() const;

protected:
	wComPtr<ID3D12Resource1> textureDefaultBuffer;
	DescriptorHeap cpuDescHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE heapCDH;
	D3D12_RESOURCE_STATES state;
	bool isRenderableTex;
};