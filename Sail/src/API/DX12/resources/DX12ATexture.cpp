#include "pch.h"
#include "DX12ATexture.h"

#include "Sail/Application.h"

DX12ATexture::DX12ATexture()
	: cpuDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Application::getInstance()->getAPI<DX12API>()->getNumSwapBuffers() * 3)
	, isRenderableTex(false)
	, useOneResource(false)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	context = Application::getInstance()->getAPI<DX12API>();
	const auto& numSwapBuffers = context->getNumSwapBuffers();

	state.resize(numSwapBuffers);
	srvHeapCDHs.resize(numSwapBuffers);
	uavHeapCDHs.resize(numSwapBuffers);
	textureDefaultBuffers.resize(numSwapBuffers);
	// Store the cpu descriptor handle that will contain the srv for this texture
	for (unsigned int i = 0; i < numSwapBuffers; i++) {
		srvHeapCDHs[i] 		= cpuDescHeap.getCPUDescriptorHandleForIndex(i * 2 + 0);
		uavHeapCDHs[i] 		= cpuDescHeap.getCPUDescriptorHandleForIndex(i * 2 + 1);
	}
}

DX12ATexture::~DX12ATexture() { }

ID3D12Resource* DX12ATexture::getResource(int swapBuffer) const {
	int i = (swapBuffer == -1) ? context->getSwapIndex() : swapBuffer;
	if (useOneResource) { i = 0; }
	return textureDefaultBuffers[i].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ATexture::getSrvCDH(int swapBuffer) const {
	int i = (swapBuffer == -1) ? context->getSwapIndex() : swapBuffer;
	if (useOneResource) { i = 0; }
	return srvHeapCDHs[i];
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ATexture::getUavCDH(int swapBuffer) const {
	int i = (swapBuffer == -1) ? context->getSwapIndex() : swapBuffer;
	if (useOneResource) { i = 0; }
	return uavHeapCDHs[i];
}

void DX12ATexture::transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState, int swapBuffer) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	int i = (swapBuffer == -1) ? context->getSwapIndex() : swapBuffer;
	auto index = (useOneResource) ? 0 : i;

	if (state[index] == newState) return;

	DX12Utils::SetResourceTransitionBarrier(cmdList, textureDefaultBuffers[index].Get(), state[index], newState);
	state[index] = newState;
}

bool DX12ATexture::isRenderable() const {
	return isRenderableTex;
}

void DX12ATexture::renameBuffer(const std::string& name) const {
	std::wstring stemp = std::wstring(name.begin(), name.end());
	LPCWSTR sw = stemp.c_str();
	for (unsigned int i = 0; i < context->getNumSwapBuffers(); i++) {
		if (textureDefaultBuffers[i]) {
			textureDefaultBuffers[i]->SetName(sw);
		}
	}
}

D3D12_RESOURCE_STATES DX12ATexture::getState(int swapBuffer) const {
	int i = (swapBuffer == -1) ? context->getSwapIndex() : swapBuffer;
	if (useOneResource) { i = 0; }

	return state[i];
}

void DX12ATexture::setState(D3D12_RESOURCE_STATES newState, int swapBuffer) {
	int i = (swapBuffer == -1) ? context->getSwapIndex() : swapBuffer;
	if (useOneResource) { i = 0; }

	state[i] = newState;
}
