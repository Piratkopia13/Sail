#pragma once

#include "../DX12API.h"

class DescriptorHeap {
public:
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors, bool shaderVisible = false);
	~DescriptorHeap();

	// An internal index behaving like a ring buffer is used
	D3D12_CPU_DESCRIPTOR_HANDLE getNextCPUDescriptorHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE getCPUDescriptorHandleForIndex(unsigned int index) const;

	// An internal index behaving like a ring buffer is used
	D3D12_GPU_DESCRIPTOR_HANDLE getNextGPUDescriptorHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUDescriptorHandleForIndex(unsigned int index) const;

	void bind(ID3D12GraphicsCommandList4* cmdList) const;

private:
	unsigned int getAndStepIndex();

private:
	wComPtr<ID3D12DescriptorHeap> m_descHeap;
	unsigned int m_incrementSize;
	unsigned int m_numDescriptors;

	unsigned int m_index;

};