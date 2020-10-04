#include "pch.h"
#include "DX12Texture.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "../shader/DX12ComputeShaderDispatcher.h"
#include "../shader/DX12PipelineStateObject.h"
#include "DDSTextureLoader/DDSTextureLoader12.cpp"

#include <filesystem>

Texture* Texture::Create(const std::string& filename, bool useAbsolutePath) {
	return SAIL_NEW DX12Texture(filename, useAbsolutePath);
}

DX12Texture::DX12Texture(const std::string& filename, bool useAbsolutePath)
	: Texture(filename)
	, m_isUploaded(false)
	, m_isInitialized(false)
	, m_initFenceVal(UINT64_MAX)
	, m_filename(filename)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<DX12API>();
	// Don't create one resource per swap buffer
	useOneResource = true;

	if (filename.substr(filename.length() - 3) == "dds") {
		// DDS textures are handled separately since they use DDSTextureLoad12 and create their own default buffer

		std::string path = (useAbsolutePath) ? filename : TextureData::DEFAULT_TEXTURE_LOCATION + filename;
		std::wstring wideFilename = std::wstring(path.begin(), path.end());
	
		bool isCubeMap;
		HRESULT hr = DirectX::LoadDDSTextureFromFile(m_context->getDevice(), wideFilename.c_str(), &textureDefaultBuffers[0], m_ddsData, m_subresources, 0, nullptr, &isCubeMap);
		if (FAILED(hr)) {
			assert(false && "DDS texture loading failed");
		}
		textureDefaultBuffers[0]->SetName((std::wstring(L"DDS texture default buffer for ") + wideFilename).c_str());
		// Tell parent class what state the resource currently is in
		state[0] = D3D12_RESOURCE_STATE_COPY_DEST;
		// No DDS texture will get auto-generated mip maps, if mip maps are wanted than they should be saved in the DDS file
		m_generateMipMaps = false;

		m_textureDesc = textureDefaultBuffers[0]->GetDesc();
		// Create a null descriptor SRV, this way the texture can be bound before it has been initialized but will show as black
		createSRV(true);
	} else {
		// Load file using the resource manager
		auto* texData = &getTextureData(filename, useAbsolutePath);

		m_textureDesc = {};
		m_textureDesc.Format = ConvertToDXGIFormat(texData->getFormat());
		m_textureDesc.Width = texData->getWidth();
		m_textureDesc.Height = texData->getHeight();
		m_textureDesc.DepthOrArraySize = 1;
		m_textureDesc.SampleDesc.Count = 1;
		m_textureDesc.SampleDesc.Quality = 0;
		m_textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		m_textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		m_generateMipMaps = true;
		// Calculate how many mip levels to generate
		DWORD mipCount;
		_BitScanForward(&mipCount, (m_textureDesc.Width == 1 ? m_textureDesc.Height : m_textureDesc.Width) | (m_textureDesc.Height == 1 ? m_textureDesc.Width : m_textureDesc.Height));
		m_textureDesc.MipLevels = mipCount + 1;
		//m_textureDesc.MipLevels = 1;

		// A texture rarely updates its data, if at all, so it is stored in a default heap
		state[0] = D3D12_RESOURCE_STATE_COPY_DEST;
		ThrowIfFailed(m_context->getDevice()->CreateCommittedResource(&DX12Utils::sDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &m_textureDesc, state[0], nullptr, IID_PPV_ARGS(&textureDefaultBuffers[0])));
		textureDefaultBuffers[0]->SetName((std::wstring(L"Texture default buffer for ") + std::wstring(filename.begin(), filename.end())).c_str());

		auto& textureData = m_subresources.emplace_back();
		textureData.pData = texData->getData();
		textureData.RowPitch = texData->getWidth() * texData->getBytesPerPixel();
		textureData.SlicePitch = textureData.RowPitch * texData->getHeight();

		// Create a null descriptor SRV, this way the texture can be bound before it has been initialized but will show as black
		createSRV(true);
	}

	// Make sure the texture is initialized on an open command list before its first use
	m_context->scheduleResourceForInit([this](ID3D12GraphicsCommandList4* initCmdlist) {
		return initBuffers(initCmdlist);
	});
}

DX12Texture::~DX12Texture() { }

bool DX12Texture::initBuffers(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// This method is called at least once every frame that this texture is used for rendering

	// The lock_guard will make sure multiple threads wont try to initialize the same texture
	std::lock_guard<std::mutex> lock(m_initializeMutex);

	if (m_isUploaded) {
		// Release the upload heap as soon as the texture has been uploaded to the GPU
		if (m_textureUploadBuffer && m_queueUsedForUpload->getCompletedFenceValue() > m_initFenceVal) {
			m_textureUploadBuffer.ReleaseAndGetAddressOf();

			// Create the correct srv (not null)
			createSRV(false);

			if (m_generateMipMaps && m_textureDesc.MipLevels > 1) {
				// Generate mip maps
				// This waits until the texture data is uploaded to the default heap since the generator reads from that source
				DX12Utils::SetResourceUAVBarrier(cmdList, textureDefaultBuffers[0].Get());
				generateMips(cmdList);
			}

			// Delete texture data from the CPU
			Application::getInstance()->getResourceManager().releaseTextureData(m_filename);
			m_ddsData.reset();

			// Fully initialized
			m_isInitialized = true;
			return true;
		}
		// Uploaded but not fully initialized with mip maps
		return false;
	}

	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	m_context->getDevice()->GetCopyableFootprints(&m_textureDesc, 0, m_subresources.size(), 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	// Create the upload heap
	// This could be done in a buffer manager owned by dx12api
	m_textureUploadBuffer.Attach(DX12Utils::CreateBuffer(m_context->getDevice(), textureUploadBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties));
	m_textureUploadBuffer->SetName(L"Texture upload buffer");
	
	// Copy the upload buffer contents to the default heap using a helper method from d3dx12.h
	DX12Utils::UpdateSubresources(cmdList, textureDefaultBuffers[0].Get(), m_textureUploadBuffer.Get(), 0, 0, m_subresources.size(), &m_subresources[0]);
	//transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); // Uncomment if generateMips is disabled

	auto type = cmdList->GetType();
	m_queueUsedForUpload = (type == D3D12_COMMAND_LIST_TYPE_DIRECT) ? m_context->getDirectQueue() : nullptr; // TODO: add support for other cmd list types
	// Schedule a signal to be called directly after this command list has executed
	// This allows us to know when the texture has been uploaded to the GPU
	m_queueUsedForUpload->scheduleSignal([this](UINT64 signaledValue) {
		m_initFenceVal = signaledValue;
	});
	
	m_isUploaded = true;
	return false;
}

void DX12Texture::createSRV(bool nullDescriptor) {
	// Create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_textureDesc.Format;

	bool isCubeMap = m_textureDesc.DepthOrArraySize == 6;

	if (isCubeMap) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = m_textureDesc.MipLevels;
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = m_textureDesc.MipLevels;
	}
	m_context->getDevice()->CreateShaderResourceView((nullDescriptor) ? nullptr : textureDefaultBuffers[0].Get(), &srvDesc, srvHeapCDHs[0]);

	// Don't allow UAV access
	uavHeapCDHs[0] = { 0 };
}

bool DX12Texture::hasBeenInitialized() const {
	return m_isInitialized;
}
bool DX12Texture::hasBeenUploaded() const {
	return m_isUploaded;
}

ID3D12Resource* DX12Texture::getResource() const {
	return textureDefaultBuffers[0].Get();
}

DXGI_FORMAT DX12Texture::ConvertToDXGIFormat(ResourceFormat::TextureFormat format) {
	DXGI_FORMAT dxgiFormat;
	switch (format) {
	case ResourceFormat::R8:
		dxgiFormat = DXGI_FORMAT_R8_UNORM;
		break;
	case ResourceFormat::R8G8:
		dxgiFormat = DXGI_FORMAT_R8G8_UNORM;
		break;
	case ResourceFormat::R8G8B8A8:
		dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case ResourceFormat::R16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16_FLOAT;
		break;
	case ResourceFormat::R16G16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16G16_FLOAT;
		break;
	case ResourceFormat::R16G16B16A16_FLOAT:
		dxgiFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case ResourceFormat::R32G32B32A32_FLOAT:
		dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	}
	return dxgiFormat;
}

const std::string& DX12Texture::getFilename() const {
	return m_filename;
}

void DX12Texture::generateMips(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	auto& mipsShader = Application::getInstance()->getResourceManager().getShaderSet(Shaders::GenerateMipsComputeShader);
	auto& settings = mipsShader.getSettings().computeShaderSettings;
	DX12ComputeShaderDispatcher csDispatcher;
	csDispatcher.begin(cmdList);

	// TODO: read this from texture data
	bool isSRGB = false;

	uint32_t srcMip = 0;

	//assert(m_textureDesc.MipLevels <= 5 && "No more than 5 mip levels can currently be generated, see commented line below to add that functionality");
	for (uint32_t srcMip = 0; srcMip < m_textureDesc.MipLevels - 1u;) {
		mipsShader.setCBufferVar("IsSRGB", &isSRGB, sizeof(bool), cmdList);

		uint64_t srcWidth = m_textureDesc.Width >> srcMip;
		uint32_t srcHeight = m_textureDesc.Height >> srcMip;
		uint32_t dstWidth = static_cast<uint32_t>(srcWidth >> 1);
		uint32_t dstHeight = srcHeight >> 1;

		// 0b00(0): Both width and height are even.
		// 0b01(1): Width is odd, height is even.
		// 0b10(2): Width is even, height is odd.
		// 0b11(3): Both width and height are odd.
		unsigned int srcDimension = (srcHeight & 1) << 1 | (srcWidth & 1);
		mipsShader.setCBufferVar("SrcDimension", &srcDimension, sizeof(unsigned int), cmdList);

		// How many mipmap levels to compute this pass (max 4 mips per pass)
		DWORD mipCount;

		// The number of times we can half the size of the texture and get
		// exactly a 50% reduction in size.
		// A 1 bit in the width or height indicates an odd dimension.
		// The case where either the width or the height is exactly 1 is handled
		// as a special case (as the dimension does not require reduction).
		_BitScanForward(&mipCount, (dstWidth == 1 ? dstHeight : dstWidth) | (dstHeight == 1 ? dstWidth : dstHeight));
		// Maximum number of mips to generate is 4.
		mipCount = std::min<DWORD>(4, mipCount + 1);
		// Clamp to total number of mips left over.
		mipCount = (srcMip + mipCount) >= m_textureDesc.MipLevels ? m_textureDesc.MipLevels - srcMip - 1 : mipCount;

		// Dimensions should not reduce to 0.
		// This can happen if the width and height are not the same.
		dstWidth = std::max<DWORD>(1, dstWidth);
		dstHeight = std::max<DWORD>(1, dstHeight);

		glm::vec2 texelSize = glm::vec2(1.0f / (float)dstWidth, 1.0f / (float)dstHeight);

		mipsShader.setCBufferVar("SrcMipLevel", &srcMip, sizeof(unsigned int), cmdList);
		mipsShader.setCBufferVar("NumMipLevels", &mipCount, sizeof(unsigned int), cmdList);
		mipsShader.setCBufferVar("TexelSize", &texelSize, sizeof(glm::vec2), cmdList);

		const auto& heap = m_context->getMainGPUDescriptorHeap();
		DescriptorHeap::DescriptorTableInstanceBuilder instance;

		// Bind source mip to t0
		transitionStateTo(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		instance.add("t0", [&](auto cpuHandle) {
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = m_textureDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = srcMip;
			srvDesc.Texture2D.MipLevels = 1;

			m_context->getDevice()->CreateShaderResourceView(textureDefaultBuffers[0].Get(), &srvDesc, cpuHandle);
		});

		// Bind output mips to u10+
		for (uint32_t mip = 0; mip < mipCount; mip++) {
			instance.add("u1" + std::to_string(mip), [&, mip](auto cpuHandle) {
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = m_textureDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = srcMip + mip + 1;
				m_context->getDevice()->CreateUnorderedAccessView(textureDefaultBuffers[0].Get(), nullptr, &uavDesc, cpuHandle);

				DX12Utils::SetResourceTransitionBarrier(cmdList, textureDefaultBuffers[0].Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip + mip + 1);
			});
		}

		// Add the instance to the heap
		// This binds resource views in the right places in the heap according to the root signature
		heap->addAndBind(instance, cmdList, true);

		// TODO: Pad any unused mip levels with a default UAV. Doing this keeps the DX12 runtime happy.
		/*if (mipCount < 4) {
			m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(GenerateMips::OutMip, mipCount, 4 - mipCount, m_GenerateMipsPSO->GetDefaultUAV());
		}*/

		// Dispatch compute shader to generate mip levels
		unsigned int x = (unsigned int)glm::ceil(dstWidth * settings.threadGroupXScale);
		unsigned int y = (unsigned int)glm::ceil(dstHeight * settings.threadGroupYScale);
		unsigned int z = 1;
		csDispatcher.dispatch(mipsShader, { x, y, z }, cmdList);

		// Transition subresources back
		for (uint32_t mip = 0; mip < mipCount; mip++) {
			DX12Utils::SetResourceTransitionBarrier(cmdList, textureDefaultBuffers[0].Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, mip + srcMip + 1);
		}

		srcMip += mipCount;
	}
	DX12Utils::SetResourceUAVBarrier(cmdList, textureDefaultBuffers[0].Get());
}