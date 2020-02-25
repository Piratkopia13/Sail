#pragma once

#include "../DX12API.h"
#include "DXRUtils.h"
#include "../DX12Utils.h"
#include "Sail/api/Renderer.h"
#include "../shader/DX12ConstantBuffer.h"
#include "API/DX12/resources/DX12RenderableTexture.h"

class DXRBase final{
public:
	DXRBase(const std::string& shaderFilename);
	~DXRBase();

	void updateAccelerationStructures(const std::vector<Renderer::RenderCommand>& sceneGeometry, ID3D12GraphicsCommandList4* cmdList);

	void updateSceneData(Camera* cam, LightSetup* lights);

	void dispatch(DX12RenderableTexture* outputTexture, ID3D12GraphicsCommandList4* cmdList);

	//void reloadShaders();

private:
	struct AccelerationStructureBuffers {
		wComPtr<ID3D12Resource> scratch = nullptr;
		wComPtr<ID3D12Resource> result = nullptr;
		wComPtr<ID3D12Resource> instanceDesc = nullptr;    // Used only for top-level AS
		bool allowUpdate = false;
		inline void release() {
			Memory::SafeRelease(scratch);
			Memory::SafeRelease(result);
			Memory::SafeRelease(instanceDesc);
		}
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
	DX12API* m_context;

	std::vector<std::unique_ptr<DX12Utils::LargeBuffer>> m_uploadBuffer;

	std::string m_shaderFilename;
	bool m_enableSoftShadowsInShader;

	std::unique_ptr<ShaderComponent::DX12ConstantBuffer> m_sceneCB;
	//std::unique_ptr<ShaderComponent::DX12ConstantBuffer> m_meshCB;

	std::vector<std::unordered_map<Mesh*, InstanceList>> m_bottomBuffers;

	std::vector<AccelerationStructureBuffers> m_topBuffer;

	wComPtr<ID3D12StateObject> m_pipelineState;

	std::vector<DXRUtils::ShaderTableData> m_rayGenShaderTable;
	std::vector<DXRUtils::ShaderTableData> m_missShaderTable;
	std::vector<DXRUtils::ShaderTableData> m_hitGroupShaderTable;

	struct MeshHandles {
		D3D12_GPU_VIRTUAL_ADDRESS vertexBufferHandle;
		D3D12_GPU_VIRTUAL_ADDRESS indexBufferHandle;
		//D3D12_GPU_DESCRIPTOR_HANDLE textureHandles[3];
	};
	std::vector<MeshHandles> m_meshHandles[2];

	std::unique_ptr<DescriptorHeap> m_descriptorHeap;
	//D3D12_CPU_DESCRIPTOR_HANDLE m_rtHeapCPUHandle[2];
	//D3D12_GPU_DESCRIPTOR_HANDLE m_rtHeapGPUHandle[2];

	struct Resource {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle[2];
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle[2];
	};

	// DXR shader outputs
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