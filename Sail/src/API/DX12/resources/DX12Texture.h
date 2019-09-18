#pragma once

#include "Sail/resources/TextureData.h"
#include "Sail/api/Texture.h"
#include "../DX12API.h"
#include "DescriptorHeap.h"
#include <mutex>

class DX12Texture : public Texture {
public:
	DX12Texture(const std::string& filename);
	~DX12Texture();

	// initBuffers is called once during its first bind
	// It is used to create resource objects that needs an open command list
	void initBuffers(ID3D12GraphicsCommandList4* cmdList);
	bool hasBeenInitialized() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getCDH() const;

private:
	DX12API* m_context;
	TextureData& m_textureData;
	D3D12_RESOURCE_DESC m_textureDesc;
	wComPtr<ID3D12Resource1> m_textureUploadBuffer;
	wComPtr<ID3D12Resource1> m_textureDefaultBuffer;
	DescriptorHeap m_cpuDescHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_heapCDH;

	std::mutex m_initializeMutex;
	bool m_isInitialized;

};