#pragma once

#include "DXRUtils.h"
#include "../DX12Utils.h"
#include "Sail/api/Renderer.h"

namespace ShaderComponent {
	class DX12ConstantBuffer;
}
class DX12RenderableTexture;
class DX12API;

class DXRBase {
public:
	struct Settings {
		UINT maxPayloadSize = 0;
		UINT maxAttributeSize = sizeof(float) * 2;
		UINT maxRecursionDepth = 1;
	};
public:
	DXRBase(const std::string& shaderFilename, Settings settings);
	virtual ~DXRBase();

	void updateAccelerationStructures(Renderer::RenderCommandList sceneGeometry, ID3D12GraphicsCommandList4* cmdList);
	void dispatch(DX12RenderableTexture* outputTexture, ID3D12GraphicsCommandList4* cmdList);

	void recreateResources();
	void reloadShaders();

	static D3D12_GPU_VIRTUAL_ADDRESS GetTLASAddress();

protected:
	struct Resource {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle[2];
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle[2];
	};
	struct DescriptorTableData {
		Resource* resource;
		D3D12_DESCRIPTOR_RANGE_TYPE type;
		unsigned int shaderRegister;
		unsigned int space;
		unsigned int numDescriptors;
	};
	struct ConstantData {
		ShaderComponent::DX12ConstantBuffer* cbuffer;
		unsigned int shaderRegister;
		unsigned int space;
	};

protected:
	// Initializes root signatures and shader resources
	// Should be called once in the constructor of the derived class
	void init();

	// Allows the derived class to add custom input and/or output resources
	// This is called once during init() and when the window resizes
	virtual void addInitialShaderResources(DescriptorHeap* heap) = 0;

protected:
	DX12API* context;
	// Shader resources for ray gen shader
	// TODO: add allow resources from other shaders to be set as well
	std::unordered_map<std::string, DescriptorTableData> rayGenDescriptorTables;
	std::unordered_map<std::string, ConstantData> globalConstants;

private:
	// TODO: replace this struct with AccelerationStructureAddresses
	struct AccelerationStructureBuffers {
		wComPtr<ID3D12Resource> scratch = nullptr;
		wComPtr<ID3D12Resource> result = nullptr;
		bool allowUpdate = false;
		void release() {
			Memory::SafeRelease(scratch);
			Memory::SafeRelease(result);
		}
	};

	struct AccelerationStructureAddresses {
		D3D12_GPU_VIRTUAL_ADDRESS scratchGpuAddress;
		D3D12_GPU_VIRTUAL_ADDRESS resultGpuAddress;
		D3D12_GPU_VIRTUAL_ADDRESS instanceDescGpuAddress; // Used only for top-level AS
	};
	
	struct InstanceList {
		AccelerationStructureBuffers blas;
		std::vector<glm::mat3x4> instanceList;
	};

private:
	// Acceleration structures
	void createTLAS(unsigned int numInstanceDescriptors, ID3D12GraphicsCommandList4* cmdList);
	void createBLAS(const Renderer::RenderCommand& renderCommand, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags, ID3D12GraphicsCommandList4* cmdList, AccelerationStructureBuffers* sourceBufferForUpdate = nullptr);

	void updateDescriptorHeap(ID3D12GraphicsCommandList4* cmdList);
	void updateShaderTables();
	void createInitialShaderResources(bool remake = false);
	void createRaytracingPSO();

	// Root signature creation
	// TODO: create them dynamically after parsing the shader source (like ShaderPipeline does)
	void createDXRGlobalRootSignature();
	void createRayGenLocalRootSignature();
	void createHitGroupLocalRootSignature();
	void createMissLocalRootSignature();
	void createEmptyLocalRootSignature();


private:
	Settings m_settings;

	std::vector<std::unique_ptr<DX12Utils::CPUSharedBuffer>> m_uploadBuffer;
	std::vector<std::unique_ptr<DX12Utils::GPUOnlyBuffer>> m_defaultBufferUA; // Used in unordered access
	std::vector<std::unique_ptr<DX12Utils::GPUOnlyBuffer>> m_defaultBufferRTAS; // Used in raytracing acceleration structures

	std::string m_shaderFilename;

	std::vector<std::unordered_map<Mesh*, InstanceList>> m_bottomBuffers;

	static std::vector<AccelerationStructureAddresses> m_topBuffer;

	wComPtr<ID3D12StateObject> m_pipelineState;

	std::vector<DXRUtils::ShaderTableData> m_rayGenShaderTable;
	std::vector<DXRUtils::ShaderTableData> m_missShaderTable;
	std::vector<DXRUtils::ShaderTableData> m_hitGroupShaderTable;
	
	std::unique_ptr<DescriptorHeap> m_descriptorHeap;
	
	Resource m_outputResource;

	const WCHAR* m_rayGenName = L"rayGen";
	const WCHAR* m_closestHitName = L"closestHitTriangle";
	const WCHAR* m_missName = L"miss";
	const WCHAR* m_hitGroupTriangleName = L"hitGroupTriangle";
	const WCHAR* m_shadowMissName = L"shadowMiss";

	std::unique_ptr<DX12Utils::RootSignature> m_dxrGlobalRootSignature;
	std::unique_ptr<DX12Utils::RootSignature> m_localSignatureRayGen;
	std::unique_ptr<DX12Utils::RootSignature> m_localSignatureHitGroup;
	std::unique_ptr<DX12Utils::RootSignature> m_localSignatureMiss;
	std::unique_ptr<DX12Utils::RootSignature> m_localSignatureEmpty;

};