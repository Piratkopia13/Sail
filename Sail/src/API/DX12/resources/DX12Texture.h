#pragma once

#include "Sail/api/Texture.h"
#include "DX12ATexture.h"
#include "Sail/resources/TextureData.h"
#include "../DX12API.h"
#include "DescriptorHeap.h"
#include <mutex>

class DX12Texture : public Texture, public virtual DX12ATexture {
public:
	DX12Texture(const std::string& filename, bool useAbsolutePath = false);
	~DX12Texture();

	// Returns true when mip maps have been generated and the texture is ready for usage
	bool hasBeenInitialized() const;
	// Returns true when the texture is uploaded to a default buffer
	bool hasBeenUploaded() const;

	const std::string& getFilename() const;
	ID3D12Resource* getResource() const;

	// TODO: check if this method is still required
	//		 upload buffer should be release automatically when it can
	//void releaseUploadBuffer();

private:
	// It is used to create resource objects that needs an open command list
	bool initBuffers(ID3D12GraphicsCommandList4* cmdList);

	void generateMips(ID3D12GraphicsCommandList4* cmdList);

private:
	std::string m_fileName;

	DX12API* m_context;
	D3D12_RESOURCE_DESC m_textureDesc;

	wComPtr<ID3D12Resource> m_textureUploadBuffer;

	DX12API::CommandQueue* m_queueUsedForUpload;
	UINT64 m_initFenceVal;

	std::mutex m_initializeMutex;
	bool m_isUploaded;
	bool m_isInitialized;

	TextureData* m_tgaData;
};