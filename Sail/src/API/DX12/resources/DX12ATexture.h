#pragma once

#include "../DX12API.h"
#include "DescriptorHeap.h"
#include "../DX12Utils.h"

class DX12ATexture {
public:
	DX12ATexture();
	~DX12ATexture();

	ID3D12Resource* getResource(int swapBuffer = -1) const;
	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCDH(int swapBuffer = -1) const;
	D3D12_CPU_DESCRIPTOR_HANDLE getUavCDH(int swapBuffer = -1) const;
	void transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState, int swapBuffer = -1);
	bool isRenderable() const;
	void renameBuffer(const std::string& name) const;
	D3D12_RESOURCE_STATES getState(int swapBuffer = -1) const;
	void setState(D3D12_RESOURCE_STATES newState, int swapBuffer = -1);

protected:
	DX12API* context;

	std::vector<wComPtr<ID3D12Resource>> textureDefaultBuffers;
	DescriptorHeap cpuDescHeap;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> srvHeapCDHs;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> uavHeapCDHs;
	
	std::vector<D3D12_RESOURCE_STATES> state;

	bool isRenderableTex;
	bool useOneResource;
};