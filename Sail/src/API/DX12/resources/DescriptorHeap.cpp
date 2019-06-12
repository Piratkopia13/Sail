#include "pch.h"
#include "DescriptorHeap.h"
#include "Sail/Application.h"

DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors, bool shaderVisible) 
	: m_numDescriptors(numDescriptors)
	, m_index(0)
{
	DX12API* context = Application::getInstance()->getAPI<DX12API>();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = numDescriptors;
	heapDesc.Flags = (shaderVisible) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.Type = type;
	ThrowIfFailed(context->getDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descHeap)));

	m_incrementSize = context->getDevice()->GetDescriptorHandleIncrementSize(type);
}

DescriptorHeap::~DescriptorHeap() {

}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::getNextCPUDescriptorHandle() {
	return getCPUDescriptorHandleForIndex(getAndStepIndex());
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::getGPUDescriptorHandleForIndex(unsigned int index) const {
	if (index >= m_numDescriptors)
		Logger::Error("Tried to get out of bounds descriptor heap gpu handle!");
	auto heapHandle = m_descHeap->GetGPUDescriptorHandleForHeapStart();
	heapHandle.ptr += index * m_incrementSize;
	return heapHandle;
}

void DescriptorHeap::bind(ID3D12GraphicsCommandList4* cmdList) const {
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_descHeap.Get() };
	cmdList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
}

unsigned int DescriptorHeap::getAndStepIndex() {
	unsigned int i = m_index;
	m_index = (m_index + 1) % m_numDescriptors;
	return i;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::getCPUDescriptorHandleForIndex(unsigned int index) const {
	if (index >= m_numDescriptors)
		Logger::Error("Tried to get out of bounds descriptor heap cpu handle!");
	auto heapHandle = m_descHeap->GetCPUDescriptorHandleForHeapStart();
	heapHandle.ptr += index * m_incrementSize;
	return heapHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::getNextGPUDescriptorHandle() {
	return getGPUDescriptorHandleForIndex(getAndStepIndex());
}
