#include "pch.h"
#include "DX12ATexture.h"

#include "Sail/Application.h"

DX12ATexture::DX12ATexture()
	: cpuDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Application::getInstance()->getAPI<DX12API>()->getNumSwapBuffers() * 2)
	, isRenderableTex(false) 
	, useOneResource(false)
{
	context = Application::getInstance()->getAPI<DX12API>();
	const auto& numSwapBuffers = context->getNumSwapBuffers();

	srvHeapCDHs.resize(numSwapBuffers);
	uavHeapCDHs.resize(numSwapBuffers);
	states.resize(numSwapBuffers);
	textureDefaultBuffers.resize(numSwapBuffers);
	// Store the cpu descriptor handle that will contain the srv for this texture
	for (unsigned int i = 0; i < numSwapBuffers; i++) {
		srvHeapCDHs[i] = cpuDescHeap.getCPUDescriptorHandleForIndex(i * 2 + 0);
		uavHeapCDHs[i] = cpuDescHeap.getCPUDescriptorHandleForIndex(i * 2 + 1);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ATexture::getSrvCDH() const {
	if (useOneResource) { return srvHeapCDHs[0]; }
	return srvHeapCDHs[context->getFrameIndex()];
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ATexture::getUavCDH() const {
	if (useOneResource) { return uavHeapCDHs[0]; }
	return uavHeapCDHs[context->getFrameIndex()];
}

void DX12ATexture::transitionStateTo(ID3D12GraphicsCommandList4* cmdList, D3D12_RESOURCE_STATES newState) {
	auto index = (useOneResource) ? 0 : context->getFrameIndex();
	if (states[index] == newState) return;

	DX12Utils::SetResourceTransitionBarrier(cmdList, textureDefaultBuffers[index].Get(), states[index], newState);
	states[index] = newState;
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
