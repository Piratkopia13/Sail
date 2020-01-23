#pragma once

#include "Sail/api/Texture.h"
#include "DX12ATexture.h"
#include "Sail/resources/TextureData.h"
#include "../DX12API.h"
#include "DescriptorHeap.h"
#include <mutex>

class DX12Texture : public Texture, public virtual DX12ATexture {
public:
	DX12Texture(const std::string& filename);
	~DX12Texture();

	// initBuffers is called once during its first bind
	// It is used to create resource objects that needs an open command list
	void initBuffers(ID3D12GraphicsCommandList4* cmdList);
	bool hasBeenInitialized() const;
	const std::string& getFilename() const;
	ID3D12Resource* getResource() const;

	void releaseUploadBuffer();

private:
	void generateMips(ID3D12GraphicsCommandList4* cmdList);

private:
	static const unsigned int MIP_LEVELS = 1;

	std::string m_fileName;

	DX12API* m_context;
	D3D12_RESOURCE_DESC m_textureDesc;

	wComPtr<ID3D12Resource> m_textureUploadBuffer;

	DX12API::CommandQueue* m_queueUsedForUpload;
	UINT64 m_initFenceVal;

	std::mutex m_initializeMutex;
	bool m_isInitialized;

	TextureData* m_tgaData;
};