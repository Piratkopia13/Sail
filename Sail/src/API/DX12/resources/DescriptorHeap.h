#pragma once

#include "../DX12API.h"
#include <mutex>

class DescriptorHeap {
public:

	class DescriptorTableInstanceBuilder {
		friend class DescriptorHeap;
	public:
		DescriptorTableInstanceBuilder();
		void add(const std::string& rootSigSlotName, std::function<void(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)> addFunc);

	private:
		DX12API* m_context;

		// Map root signature slot to entries
		// Entires map type to a function which copies a descriptor to the given cpuHandle
		std::map<unsigned int, std::map<unsigned int, std::function<void(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)>>> m_entries;
	};

public:
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors, bool shaderVisible = false, bool frameDependant = false);
	~DescriptorHeap();

	ID3D12DescriptorHeap* get() const;

	// An internal index behaving like a ring buffer is used
	D3D12_CPU_DESCRIPTOR_HANDLE getNextCPUDescriptorHandle(int nSteps = 1);
	D3D12_CPU_DESCRIPTOR_HANDLE getCPUDescriptorHandleForIndex(unsigned int index) const;

	// An internal index behaving like a ring buffer is used
	D3D12_GPU_DESCRIPTOR_HANDLE getNextGPUDescriptorHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUDescriptorHandleForIndex(unsigned int index) const;

	D3D12_CPU_DESCRIPTOR_HANDLE	getCurentCPUDescriptorHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE	getCurentGPUDescriptorHandle() const;
	unsigned int getDescriptorIncrementSize() const;

	void setIndex(unsigned int index);
	unsigned int getCurrentIndex() const;

	// Binds resource views in the right places in the heap according to the root signature
	void addAndBind(DescriptorTableInstanceBuilder& instance, ID3D12GraphicsCommandList4* cmdList, bool onCompute = false);

	void bind(ID3D12GraphicsCommandList4* cmdList) const;
	unsigned int getAndStepIndex(int nSteps = 1);

private:
	std::mutex m_getAndStepIndex_mutex;

private:
	wComPtr<ID3D12DescriptorHeap> m_descHeap;
	unsigned int m_incrementSize;
	unsigned int m_numDescriptors;

	unsigned int m_index;

	// Frame dependent splits the descriptor heap into two equally sized parts
	// Where each half is allocated for one swap buffer
	bool m_frameDependant;
	D3D12_CPU_DESCRIPTOR_HANDLE m_secondHalfCPUHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE m_secondHalfGPUHandleStart;
	DX12API* m_context;
};