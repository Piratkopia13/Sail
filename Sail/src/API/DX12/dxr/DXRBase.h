#pragma once

#include "../DX12API.h"
#include "DXRUtils.h"

namespace DXRGlobalRootParam {
	enum Slot {
		FLOAT_RED_CHANNEL = 0,
		SRV_ACCELERATION_STRUCTURE,
		CBV_SCENE_BUFFER,
		CBV_SETTINGS,
		SIZE
	};
}
namespace DXRRayGenRootParam {
	enum Slot {
		DT_UAV_OUTPUT = 0,
		SIZE
	};
}
namespace DXRHitGroupRootParam {
	enum Slot {
		SRV_VERTEX_BUFFER = 0,
		SRV_INDEX_BUFFER,
		DT_TEXTURES,
		CBV_MATERIAL,
		SIZE
	};
}
namespace DXRMissRootParam {
	enum Slot {
		SRV_SKYBOX = 0,
		SIZE
	};
}

class DXRBase {
public:

	// TODO: somehow allow this to change from different DXRBase instances
	struct RayPayload {
		glm::vec4 color;
		UINT recursionDepth;
		int hit;
	};

	DXRBase(const std::string& shaderFilename);
	~DXRBase();

	//void updateAccelerationStructures(ID3D12GraphicsCommandList4* cmdList);
	void dispatch(ID3D12GraphicsCommandList4* cmdList);

private:
	// Acceleration structures
	void createTLAS(ID3D12GraphicsCommandList4* cmdList);
	void createBLAS(ID3D12GraphicsCommandList4* cmdList);

	// Other DXR requirements
	void createShaderResources();
	void createShaderTables();
	void createRaytracingPSO();

	// Root signature creation
	// TODO: create them dynamically after parsing the shader source (like ShaderPipeline does)
	void createDXRGlobalRootSignature();
	ID3D12RootSignature* createRayGenLocalRootSignature();
	ID3D12RootSignature* createHitGroupLocalRootSignature();
	ID3D12RootSignature* createMissLocalRootSignature();

private:
	DX12API* m_context;
	std::string m_shaderFilename;

	static const int MAX_RAY_RECURSION_DEPTH;

	//union AlignedSceneConstantBuffer { 	// TODO: Use this instead of SceneConstantBuffer
	//	SceneConstantBuffer* constants;
	//	uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
	//};
	//AlignedSceneConstantBuffer* m_mappedSceneCBData; // TODO: Fix memory leak
	//SceneConstantBuffer* m_sceneCBData; // TODO: Fix memory leak
	//RayGenSettings m_rayGenCBData;

	struct AccelerationStructureBuffers {
		wComPtr<ID3D12Resource1> scratch = nullptr;
		wComPtr<ID3D12Resource1> result = nullptr;
		wComPtr<ID3D12Resource1> instanceDesc = nullptr;    // Used only for top-level AS
		void release() {
			if (scratch) {
				scratch->Release();
				scratch = nullptr;
			}
			if (result) {
				result->Release();
				result = nullptr;
			}
			if (instanceDesc) {
				instanceDesc->Release();
				instanceDesc = nullptr;
			}
		}
	};
	std::vector<std::vector<AccelerationStructureBuffers>> m_DXR_BottomBuffers;
	std::vector<AccelerationStructureBuffers> m_DXR_TopBuffer;

	wComPtr<ID3D12StateObject> m_rtPipelineState = nullptr;

	std::vector<DXRUtils::ShaderTableData> m_rayGenShaderTable{};
	std::vector<DXRUtils::ShaderTableData> m_missShaderTable{};
	std::vector<DXRUtils::ShaderTableData> m_hitGroupShaderTable{};

	struct ResourceWithDescriptor {
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		wComPtr<ID3D12Resource> resource;
	};
	struct MeshHandles {
		D3D12_GPU_VIRTUAL_ADDRESS vertexBufferHandle;
		D3D12_GPU_VIRTUAL_ADDRESS indexBufferHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle;
		D3D12_GPU_VIRTUAL_ADDRESS materialHandle;
	};

	wComPtr<ID3D12DescriptorHeap> m_rtDescriptorHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_rtHeapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_rtHeapGPUHandle;
	UINT m_heapIncr;
	ResourceWithDescriptor m_rtOutputUAV;

	std::vector<MeshHandles> m_rtMeshHandles;

	const WCHAR* m_rayGenName = L"rayGen";
	const WCHAR* m_closestHitName = L"closestHit";
	const WCHAR* m_missName = L"miss";
	const WCHAR* m_hitGroupName = L"HitGroup";

	wComPtr<ID3D12RootSignature> m_dxrGlobalRootSignature;
	wComPtr<ID3D12RootSignature> m_localSignatureRayGen;
	wComPtr<ID3D12RootSignature> m_localSignatureHitGroup;
	wComPtr<ID3D12RootSignature> m_localSignatureMiss;


};