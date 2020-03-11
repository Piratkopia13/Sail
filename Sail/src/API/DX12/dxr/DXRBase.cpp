#include "pch.h"
#include "DXRBase.h"
#include "Sail/Application.h"
#include "API/DX12/DX12VertexBuffer.h"
#include "API/DX12/DX12IndexBuffer.h"
#include "../resources/DX12RenderableTexture.h"
#include "../shader/DX12ConstantBuffer.h"

// Include defines shared with dxr shaders
#include "Sail/../../Demo/res/shaders/dxr/dxr.shared"
#include <bitset>
#include "Sail/graphics/material/PBRMaterial.h"

std::vector<DXRBase::AccelerationStructureAddresses> DXRBase::m_topBuffer;

DXRBase::DXRBase(const std::string& shaderFilename, Settings settings)
	: m_shaderFilename(shaderFilename)
	, m_settings(settings)
{
	context = Application::getInstance()->getAPI<DX12API>();

	// Create frame resources (one per swap buffer)
	// Only one TLAS is used for the whole scene
	m_topBuffer = context->createFrameResource<AccelerationStructureAddresses>();
	m_bottomBuffers = context->createFrameResource<std::unordered_map<Mesh*, InstanceList>>();
	m_rayGenShaderTable = context->createFrameResource<DXRUtils::ShaderTableData>();
	m_missShaderTable = context->createFrameResource<DXRUtils::ShaderTableData>();
	m_hitGroupShaderTable = context->createFrameResource<DXRUtils::ShaderTableData>();
}

void DXRBase::init() {
	// Create root signatures
	createDXRGlobalRootSignature();
	createRayGenLocalRootSignature();
	createHitGroupLocalRootSignature();
	createMissLocalRootSignature();
	createEmptyLocalRootSignature();

	createRaytracingPSO();
	createInitialShaderResources();
}

DXRBase::~DXRBase() {
	m_pipelineState->Release();
	for (auto& blasList : m_bottomBuffers) {
		for (auto& blas : blasList) {
			blas.second.blas.release();
		}
	}
}

void DXRBase::updateAccelerationStructures(const std::vector<Renderer::RenderCommand>& sceneGeometry, ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	unsigned int frameIndex = context->getSwapIndex();
	unsigned int totalNumInstances = 0;

	auto flagNone = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	auto flagFastTrace = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	auto flagFastBuild = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	auto flagAllowUpdate = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	// Clear old instance lists
	for (auto& it : m_bottomBuffers[frameIndex]) {
		it.second.instanceList.clear();
	}
	
	// Iterate all static meshes
	for (auto& renderCommand : sceneGeometry) {
		if (renderCommand.dxrFlags & Renderer::MESH_STATIC) {
			Mesh* mesh = renderCommand.mesh;
			auto& searchResult = m_bottomBuffers[frameIndex].find(mesh);
			if (searchResult == m_bottomBuffers[frameIndex].end()) {
				// If mesh does not have a BLAS
				createBLAS(renderCommand, flagFastTrace, cmdList);
			} else {
				// Mesh has a BLAS
				// TODO: uncomment and fix
				//if (renderCommand.hasUpdatedSinceLastRender[frameIndex]) {
				//	Logger::Log("A BLAS rebuild has been triggered on a STATIC mesh. Consider changing it to DYNAMIC!");
				//	// Destroy old blas
				//	searchResult->second.blas.release();
				//	m_bottomBuffers[frameIndex].erase(searchResult);
				//	// Create new one
				//	createBLAS(renderCommand, flagFastTrace, cmdList);
				//} else 
				{
					// Mesh already has a BLAS - add instance to list
					searchResult->second.instanceList.push_back({ renderCommand.material, (glm::mat3x4)renderCommand.transform });
				}
			}
			totalNumInstances++;
		}
	}

	// Iterate all dynamic meshes
	for (auto& renderCommand : sceneGeometry) {
		if (renderCommand.dxrFlags & Renderer::MESH_DYNAMIC) {
			Mesh* mesh = renderCommand.mesh;
		
			auto& searchResult = m_bottomBuffers[frameIndex].find(mesh);
			auto flags = flagNone;
			if (renderCommand.dxrFlags & Renderer::MESH_HERO) {
				flags = flagFastTrace | flagAllowUpdate;
			} else {
				flags = flagFastBuild | flagAllowUpdate;
			}

			// If mesh does not have a BLAS or was first built as STATIC
			if (searchResult == m_bottomBuffers[frameIndex].end() || !searchResult->second.blas.allowUpdate) {
				createBLAS(renderCommand, flags, cmdList);
			} else {
				// TODO: uncomment and fix
				/*if (renderCommand.hasUpdatedSinceLastRender[frameIndex]) {
					createBLAS(renderCommand, flags, cmdList, &searchResult->second.blas);
				}*/
				// Add transform to instance list
				searchResult->second.instanceList.push_back({ renderCommand.material, (glm::mat3x4)renderCommand.transform });
			}

			totalNumInstances++;
		}
	}

	// Destroy BLASes that are no longer part of the scene
	for (auto it = m_bottomBuffers[frameIndex].begin(); it != m_bottomBuffers[frameIndex].end();) {
		bool destroy = true;
		for (auto& renderCommand : sceneGeometry) {
			Mesh* mesh = renderCommand.mesh;

			if (it->first == mesh) {
				destroy = false;
				++it;
				break;
			}
		}
		if (destroy) {
			it->second.blas.release();
			it = m_bottomBuffers[frameIndex].erase(it);
		}
	}

	// Any following allocations should start from the beginning of the uploadBuffer and overwrite existing tlas instances, shader tables and mesh data
	m_uploadBuffer[frameIndex]->setCurrentPointerOffset(0);
	m_defaultBufferUA[frameIndex]->setCurrentPointerOffset(0);
	m_defaultBufferRTAS[frameIndex]->setCurrentPointerOffset(0);

	createTLAS(totalNumInstances, cmdList);
	updateDescriptorHeap(cmdList);
	updateShaderTables();
}

void DXRBase::dispatch(const DX12RenderableTexture* const* outputTextures, unsigned int numOutputTextures, ID3D12GraphicsCommandList4* cmdList) {
	unsigned int frameIndex = context->getSwapIndex();
	assert(m_settings.numOutputTextures >= numOutputTextures && "Too many output textures passed");

	auto copyDescriptor = [&](const DX12RenderableTexture* texture, D3D12_CPU_DESCRIPTOR_HANDLE cdh) {
		// Copy output texture uav to heap
		//texture->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
		context->getDevice()->CopyDescriptorsSimple(1, cdh, texture->getUavCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	};
	auto handle = m_outputFirstResource.cpuHandle[frameIndex];
	copyDescriptor(outputTextures[0], handle);
	for (unsigned int i = 1; i < numOutputTextures; i++) {
		handle.ptr += m_descriptorHeap->getDescriptorIncrementSize();
		copyDescriptor(outputTextures[i], handle);
	}
	
	// Set constant buffer descriptor heap
	m_descriptorHeap->bind(cmdList);

	// Let's raytrace

	D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
	raytraceDesc.Width = Application::getInstance()->getWindow()->getWindowWidth();
	raytraceDesc.Height = Application::getInstance()->getWindow()->getWindowHeight();
	raytraceDesc.Depth = 1;

	//set shader tables
	raytraceDesc.RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable[frameIndex].gpuAddress;
	raytraceDesc.RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable[frameIndex].SizeInBytes;

	raytraceDesc.MissShaderTable.StartAddress = m_missShaderTable[frameIndex].gpuAddress;
	raytraceDesc.MissShaderTable.StrideInBytes = m_missShaderTable[frameIndex].StrideInBytes;
	raytraceDesc.MissShaderTable.SizeInBytes = m_missShaderTable[frameIndex].SizeInBytes;

	raytraceDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable[frameIndex].gpuAddress;
	raytraceDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTable[frameIndex].StrideInBytes;
	raytraceDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable[frameIndex].SizeInBytes;

	// Bind the global root signature
	cmdList->SetComputeRootSignature(*m_dxrGlobalRootSignature->get());

	// Bind acceleration structure
	cmdList->SetComputeRootShaderResourceView(m_dxrGlobalRootSignature->getIndex("AccelerationStructure"), m_topBuffer[frameIndex].resultGpuAddress);

	// Bind constants specified in derived class
	for (auto& it : globalConstants) {
		cmdList->SetComputeRootConstantBufferView(m_dxrGlobalRootSignature->getIndex(it.first), it.second.cbuffer->getBuffer()->GetGPUVirtualAddress());
	}

	// Dispatch
	cmdList->SetPipelineState1(m_pipelineState.Get());
	cmdList->DispatchRays(&raytraceDesc);
}

void DXRBase::reloadShaders() {
	context->waitForGPU();
	// Recompile hlsl
	createRaytracingPSO();
}

void DXRBase::createTLAS(unsigned int numInstanceDescriptors, ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// Always rebuilds TLAS instead of updating it according to nvidia recommendations

	unsigned int frameIndex = context->getSwapIndex();

	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = numInstanceDescriptors;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	void* mappedInstanceBuffer;
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Buffers re-creation");

		// Re-create the buffer
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
		context->getDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		// Allocate new memory regions for buffers
		// Program will crash if a buffer is too small for a suballocation
		// TODO: Somehow allow resizing buffers when needed without breaking GPU_VIRTUAL_ADDRESS references to the current buffer
		m_topBuffer[frameIndex].scratchGpuAddress = m_defaultBufferUA[frameIndex]->suballocate(info.ScratchDataSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		m_topBuffer[frameIndex].resultGpuAddress = m_defaultBufferRTAS[frameIndex]->suballocate(info.ResultDataMaxSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		m_topBuffer[frameIndex].instanceDescGpuAddress = m_uploadBuffer[frameIndex]->suballocate(numInstanceDescriptors * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT, &mappedInstanceBuffer);
	}

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Write blas instances");

		D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc = (D3D12_RAYTRACING_INSTANCE_DESC*)mappedInstanceBuffer;

		unsigned int blasIndex = 0;
		for (auto& it : m_bottomBuffers[frameIndex]) {
			unsigned int instanceIndex = 0;
			auto& instanceList = it.second;
			for (auto& instance : instanceList.instanceList) {
				auto& transform = instance.transform;
				// InstanceID will store both the blasIndex and the instanceIndex, both with 16 bits
				std::bitset<sizeof(unsigned int) * 8> blasIdBitsetTest(blasIndex >> 16);
				assert(blasIdBitsetTest.none() && "blasIndex is too big! Reduce the number of unique meshes in the scene or change the bit logic below");
				std::bitset<sizeof(unsigned int) * 8> instanceIdBitsetTest(instanceIndex >> 16);
				assert(instanceIdBitsetTest.none() && "instanceIndex is too big! Reduce the number of instances of this mesh in the scene or change the bit logic below");
				pInstanceDesc->InstanceID = blasIndex | (instanceIndex << 16);
				pInstanceDesc->InstanceMask = 0xFF;
				pInstanceDesc->InstanceContributionToHitGroupIndex = blasIndex * 2;	// offset inside the shader-table. Unique for every instance since each geometry has different vertexbuffer/indexbuffer/textures
																					// * 2 since every other entry in the SBT is for shadow rays (NULL hit group)
				pInstanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
				memcpy(pInstanceDesc->Transform, &transform, sizeof(pInstanceDesc->Transform));
				pInstanceDesc->AccelerationStructure = instanceList.blas.result->GetGPUVirtualAddress();
				pInstanceDesc++;
				instanceIndex++;
			}
			blasIndex++;
		}
	}

	// Create the TLAS
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Build TLAS");

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
		asDesc.Inputs = inputs;
		asDesc.Inputs.InstanceDescs = m_topBuffer[frameIndex].instanceDescGpuAddress;
		asDesc.DestAccelerationStructureData = m_topBuffer[frameIndex].resultGpuAddress;
		asDesc.ScratchAccelerationStructureData = m_topBuffer[frameIndex].scratchGpuAddress;

		cmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);
	}

	// UAV barrier needed before using the acceleration structures in a raytracing operation
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = m_defaultBufferRTAS[frameIndex]->getResource();
	cmdList->ResourceBarrier(1, &uavBarrier);
}

void DXRBase::createBLAS(const Renderer::RenderCommand& renderCommand, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags, ID3D12GraphicsCommandList4* cmdList, AccelerationStructureBuffers* sourceBufferForUpdate) {
	unsigned int frameIndex = context->getSwapIndex();
	Mesh* mesh = renderCommand.mesh;

	bool performInplaceUpdate = (sourceBufferForUpdate) ? true : false;

	InstanceList instance;
	instance.instanceList.push_back({ renderCommand.material, renderCommand.transform });
	AccelerationStructureBuffers& bottomBuffer = instance.blas;
	if (performInplaceUpdate) {
		bottomBuffer = *sourceBufferForUpdate;
	}
	if (flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) {
		instance.blas.allowUpdate = true;
	}

	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	auto& vb = static_cast<DX12VertexBuffer&>(mesh->getVertexBuffer());
	auto& ib = static_cast<DX12IndexBuffer&>(mesh->getIndexBuffer());

	// Make sure buffer is initialized
	vb.init(cmdList);
	// Transition to correct state for as building
	DX12Utils::SetResourceTransitionBarrier(cmdList, vb.getResource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	geomDesc.Flags = (renderCommand.dxrFlags & Renderer::MESH_TRANSPARENT) ? D3D12_RAYTRACING_GEOMETRY_FLAG_NONE : D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Triangles.VertexBuffer.StartAddress = vb.getResource()->GetGPUVirtualAddress();
	geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Mesh::vec3);
	geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geomDesc.Triangles.VertexCount = mesh->getNumVertices();

	if (mesh->getNumIndices() > 0) {
		// Make sure buffer is initialized
		ib.init(cmdList);
		// Transition to correct state for as building
		DX12Utils::SetResourceTransitionBarrier(cmdList, ib.getResource(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		geomDesc.Triangles.IndexBuffer = ib.getResource()->GetGPUVirtualAddress();
		geomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geomDesc.Triangles.IndexCount = UINT(mesh->getNumIndices());
	}


	// Get the size requirements for the scratch and AS buffers
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = flags; // Changing this flag depending on mesh can speed up performance significantly!
	if (performInplaceUpdate) {
		inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	}
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomDesc;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	context->getDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	// TODO: make sure buffer size is >= info.UpdateScratchDataSize in bytes
	if (!performInplaceUpdate) {
		// Create the buffers. They need to support UAV, and since we are going to immediately use them, we create them with an unordered-access state
		// TODO: replace following CreateBuffer with suballocations from a larger buffer
		bottomBuffer.scratch = DX12Utils::CreateBuffer(context->getDevice(), info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::sDefaultHeapProps);
		bottomBuffer.scratch->SetName(L"BLAS_SCRATCH");
		bottomBuffer.result = DX12Utils::CreateBuffer(context->getDevice(), info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, DX12Utils::sDefaultHeapProps);
		bottomBuffer.result->SetName(L"BLAS_RESULT");
	}

	// Create the bottom-level AS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.ScratchAccelerationStructureData = bottomBuffer.scratch->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = bottomBuffer.result->GetGPUVirtualAddress();
	if (inputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE) {
		asDesc.SourceAccelerationStructureData = sourceBufferForUpdate->result->GetGPUVirtualAddress();
	}

	cmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// Transition index and vertex buffers back to be used in non-raytracing operations
	DX12Utils::SetResourceTransitionBarrier(cmdList, vb.getResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	if (mesh->getNumIndices() > 0)
		DX12Utils::SetResourceTransitionBarrier(cmdList, ib.getResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	// We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = bottomBuffer.result.Get();
	cmdList->ResourceBarrier(1, &uavBarrier);

	if (!performInplaceUpdate) {
		// Insert BLAS into bottom buffer map
		m_bottomBuffers[frameIndex].insert({ mesh, instance });
	}
}

void DXRBase::createInitialShaderResources(bool remake) {
	// Create some resources only once on init
	if (!m_descriptorHeap || remake) {
		UINT numDescriptors = 500;
		m_descriptorHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescriptors, true, true);

		// The first slots in the heap will be used for resources that never changes
		auto storeOutputHandle = [&](Resource& res) {
			for (unsigned int i = 0; i < 2; i++) {
				auto index = m_descriptorHeap->getAndStepIndex();
				res.cpuHandle[i] = m_descriptorHeap->getCPUDescriptorHandleForIndex(index);
				res.gpuHandle[i] = m_descriptorHeap->getGPUDescriptorHandleForIndex(index);
				// Leave heap slots for the rest of the output textures
				m_descriptorHeap->getAndStepIndex(m_settings.numOutputTextures-1);
			}
		};
		// UAV output start
		storeOutputHandle(m_outputFirstResource);

		// Add resources from derived class
		addInitialShaderResources(m_descriptorHeap.get());

		// Store the index to the descriptor heap position where data can be changed in runtime
		m_heapDynamicStartIndex = m_descriptorHeap->getCurrentIndex();

		// TODO: tweak these
		unsigned int uploadBufferByteSize = 1024 * 100;
		unsigned int defaultBufferUAByteSize = 1024 * 20;
		unsigned int defaultBufferRTASByteSize = 1024 * 20;
		for (unsigned int i = 0; i < context->getNumGPUBuffers(); i++) {
			m_defaultBufferUA.emplace_back(new DX12Utils::GPUOnlyBuffer(context->getDevice(), defaultBufferUAByteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			m_defaultBufferRTAS.emplace_back(new DX12Utils::GPUOnlyBuffer(context->getDevice(), defaultBufferRTASByteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE));
			m_uploadBuffer.emplace_back(new DX12Utils::CPUSharedBuffer(context->getDevice(), uploadBufferByteSize));
		}
	}
}

void DXRBase::updateDescriptorHeap(ID3D12GraphicsCommandList4* cmdList) {
	// Return instantly if the everything the method does is turned off in settings
	if (!(m_settings.bindVertexBuffers || m_settings.bindTextures)) return;


	// TODO: profile performance of this method and optimize where possible

	unsigned int frameIndex = context->getSwapIndex();

	// Update descriptors for vertices, indices, textures etc
	//D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_descriptorHeap->getCPUDescriptorHandleForIndex(m_heapDynamicStartIndex);
	//D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_descriptorHeap->getGPUDescriptorHandleForIndex(m_heapDynamicStartIndex);
	m_descriptorHeap->setIndex(m_heapDynamicStartIndex);

	m_positionsBufferHandles[frameIndex].clear();
	m_texCoordsBufferHandles[frameIndex].clear();
	m_normalsBufferHandles[frameIndex].clear();
	m_tangentsBufferHandles[frameIndex].clear();
	m_bitangentsBufferHandles[frameIndex].clear();
	m_indexBufferHandles[frameIndex].clear();
	m_textureHandles[frameIndex].clear();
	m_meshDataBuffers[frameIndex].clear();

	
	unsigned int blasIndex = 0;
	for (auto& it : m_bottomBuffers[frameIndex]) {
		Mesh* mesh = it.first;
		auto& instanceList = it.second.instanceList;

		auto vertexBufferStart = static_cast<const DX12VertexBuffer&>(mesh->getVertexBuffer()).getResource()->GetGPUVirtualAddress();
		m_positionsBufferHandles[frameIndex].emplace_back(vertexBufferStart + mesh->getVertexBuffer().getPositionsOffset());
		m_texCoordsBufferHandles[frameIndex].emplace_back(vertexBufferStart + mesh->getVertexBuffer().getTexCoordsOffset());
		m_normalsBufferHandles[frameIndex].emplace_back(vertexBufferStart + mesh->getVertexBuffer().getNormalsOffset());
		m_tangentsBufferHandles[frameIndex].emplace_back(vertexBufferStart + mesh->getVertexBuffer().getTangentsOffset());
		m_bitangentsBufferHandles[frameIndex].emplace_back(vertexBufferStart + mesh->getVertexBuffer().getBitangentsOffset());
		if (mesh->getNumIndices() > 0) {
			m_indexBufferHandles[frameIndex].emplace_back(static_cast<const DX12IndexBuffer&>(mesh->getIndexBuffer()).getResource()->GetGPUVirtualAddress());
		} else {
			// Emplace empty handle to keep order intact
			m_indexBufferHandles[frameIndex].emplace_back();
		}

		// Material textures are bound to unbounded arrays, order is important which is why we first iterate through the instances three times
		// Textures needs to be stored in the heap like 
		// instance_1_albedo, instance_.._albedo, instance_n_albedo, instance_1_normal, instance_.._normal, instance_n_normal, instance_1_mrao, instance_.._mrao, instance_n_mrao, 
		m_textureHandles[frameIndex].emplace_back();
		bool hasSetTexture = false;
		auto bindTextures = [&](unsigned int textureNum, PBRMaterial* material) {
			auto& materialSettings = material->getPBRSettings();
			bool hasTexture = (textureNum == 0) ? materialSettings.hasAlbedoTexture : materialSettings.hasNormalTexture;
			hasTexture = (textureNum == 2) ? materialSettings.hasMetalnessRoughnessAOTexture : hasTexture;

			// Increase pointer regardless of if the texture existed or not to keep to order in the SBT
			auto descIndex = m_descriptorHeap->getAndStepIndex();
			if (hasTexture) {
				DX12Texture* texture = static_cast<DX12Texture*>(material->getTexture(textureNum));
				// Copy SRV to DXR heap
				context->getDevice()->CopyDescriptorsSimple(1, m_descriptorHeap->getCPUDescriptorHandleForIndex(descIndex), texture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			if (!hasSetTexture) {
				// Only store the handle to the texture of the first instance since textures are read through an array in the shader
				m_textureHandles[frameIndex][blasIndex][textureNum] = m_descriptorHeap->getGPUDescriptorHandleForIndex(descIndex);
				hasSetTexture = true;
			}
		};
		for (const auto& instance : instanceList) {
			PBRMaterial* mat = dynamic_cast<PBRMaterial*>(instance.material);
			assert(mat && "All materials for raytraced meshes must be PBRMaterial!");
			bindTextures(0, mat); // Albedo
		}
		hasSetTexture = false;
		for (const auto& instance : instanceList) {
			PBRMaterial* mat = dynamic_cast<PBRMaterial*>(instance.material);
			bindTextures(1, mat); // Normal map
		}
		hasSetTexture = false;
		for (const auto& instance : instanceList) {
			PBRMaterial* mat = dynamic_cast<PBRMaterial*>(instance.material);
			bindTextures(2, mat); // MRAO
		}

		unsigned int meshDataSize = sizeof(DXRShaderCommon::InstanceData) * instanceList.size();
		std::vector<DXRShaderCommon::InstanceData> meshData(instanceList.size());
		unsigned int i = 0;
		for (const auto& instance : instanceList) {
			PBRMaterial* mat = dynamic_cast<PBRMaterial*>(instance.material);
			auto& materialSettings = mat->getPBRSettings();

			// Update per mesh data
			// Such as flags telling the shader to use indices, textures or not
			
			meshData[i].flags = (mesh->getNumIndices() == 0) ? DXRShaderCommon::MESH_NO_FLAGS : DXRShaderCommon::MESH_USE_INDICES;
			meshData[i].flags |= (materialSettings.hasAlbedoTexture) ? DXRShaderCommon::MESH_HAS_ALBEDO_TEX : meshData[i].flags;
			meshData[i].flags |= (materialSettings.hasNormalTexture) ? DXRShaderCommon::MESH_HAS_NORMAL_TEX : meshData[i].flags;
			meshData[i].flags |= (materialSettings.hasMetalnessRoughnessAOTexture) ? DXRShaderCommon::MESH_HAS_MRAO_TEX: meshData[i].flags;
			meshData[i].color = materialSettings.modelColor;
			meshData[i].metalnessScale = materialSettings.metalnessScale;
			meshData[i].roughnessScale = materialSettings.roughnessScale;
			meshData[i].aoIntensity = materialSettings.aoIntensity;
			i++;
		}
		m_meshDataBuffers[frameIndex].emplace_back(m_uploadBuffer[frameIndex]->setData(meshData.data(), meshDataSize, 0U));
		
		blasIndex++;
	}

}

void DXRBase::updateShaderTables() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	// 	 "Shader tables can be modified freely by the application (with appropriate state barriers)"

	auto frameIndex = context->getSwapIndex();

	// Ray gen
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Ray gen");

		DXRUtils::ShaderTableBuilder tableBuilder(1U, m_pipelineState.Get(), 96U);
		tableBuilder.addShader(m_rayGenName);
		m_localSignatureRayGen->doInOrder([&](const std::string& parameterName) {
			if (rayGenDescriptorTables.find(parameterName) != rayGenDescriptorTables.end()) {
				auto& dt = rayGenDescriptorTables[parameterName];
				tableBuilder.addDescriptor(dt.resource->gpuHandle[frameIndex].ptr);
			} else if(parameterName == "OutputUAVs") {
				tableBuilder.addDescriptor(m_outputFirstResource.gpuHandle[frameIndex].ptr);
			} else {
				Logger::Error("Unhandled root signature parameter! (" + parameterName + ")");
			}
		});
		
		{
			SAIL_PROFILE_API_SPECIFIC_SCOPE("Build");
			m_rayGenShaderTable[frameIndex] = tableBuilder.build(context->getDevice(), *m_uploadBuffer[frameIndex]);
		}
	}

	// Miss
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Miss");

		DXRUtils::ShaderTableBuilder tableBuilder(2U, m_pipelineState.Get());
		tableBuilder.addShader(m_missName);
		tableBuilder.addShader(m_shadowMissName);
		{
			SAIL_PROFILE_API_SPECIFIC_SCOPE("Build");
			m_missShaderTable[frameIndex] = tableBuilder.build(context->getDevice(), *m_uploadBuffer[frameIndex]);
		}
	}

	// Hit group
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Hit group");

		UINT numInstances = (UINT)m_bottomBuffers[frameIndex].size() * 2U; // * 2 for shadow rays (all NULL)
		DXRUtils::ShaderTableBuilder tableBuilder(numInstances, m_pipelineState.Get(), 96U);

		unsigned int blasIndex = 0;

		for (auto& it : m_bottomBuffers[frameIndex]) {
			Mesh* mesh = it.first;
			unsigned int inst = blasIndex * 2;

			tableBuilder.addShader(m_hitGroupTriangleName); // Set the shader-group to use
			m_localSignatureHitGroup->doInOrder([&](const std::string& parameterName) {
				if (parameterName == "PositionsBuffer") {
					tableBuilder.addDescriptor(m_positionsBufferHandles[frameIndex][blasIndex], inst);
				} else if (parameterName == "TexCoordsBuffer") {
					tableBuilder.addDescriptor(m_texCoordsBufferHandles[frameIndex][blasIndex], inst);
				} else if (parameterName == "NormalsBuffer") {
					tableBuilder.addDescriptor(m_normalsBufferHandles[frameIndex][blasIndex], inst);
				} else if (parameterName == "TangentsBuffer") {
					tableBuilder.addDescriptor(m_tangentsBufferHandles[frameIndex][blasIndex], inst);
				} else if (parameterName == "BitangentsBuffer") {
					tableBuilder.addDescriptor(m_bitangentsBufferHandles[frameIndex][blasIndex], inst);
				} else if (parameterName == "IndexBuffer") {
					D3D12_GPU_VIRTUAL_ADDRESS nullAddr = 0;
					tableBuilder.addDescriptor((mesh->getNumIndices() > 0) ? m_indexBufferHandles[frameIndex][blasIndex] : nullAddr, inst);
				} else if (parameterName == "MeshData") {
					D3D12_GPU_VIRTUAL_ADDRESS meshDataHandle = m_meshDataBuffers[frameIndex][blasIndex];
					tableBuilder.addDescriptor(meshDataHandle, inst);
				} else if (parameterName == "TexturesAlbedo") {
					tableBuilder.addDescriptor(m_textureHandles[frameIndex][blasIndex][0].ptr, inst);
				} else if (parameterName == "TexturesNormal") {
					tableBuilder.addDescriptor(m_textureHandles[frameIndex][blasIndex][1].ptr, inst);
				} else if (parameterName == "TexturesMRAO") {
					tableBuilder.addDescriptor(m_textureHandles[frameIndex][blasIndex][2].ptr, inst);
				} else {
					Logger::Error("Unhandled root signature parameter! (" + parameterName + ")");
				}
			});
			
			tableBuilder.addShader(L"NULL");
			blasIndex++;
		}
		{
			SAIL_PROFILE_API_SPECIFIC_SCOPE("Build");
			m_hitGroupShaderTable[frameIndex] = tableBuilder.build(context->getDevice(), *m_uploadBuffer[frameIndex]);
		}
	}
}

void DXRBase::createRaytracingPSO() {
	Memory::SafeRelease(m_pipelineState);

	DXRUtils::PSOBuilder psoBuilder;

	psoBuilder.addLibrary(Shader::DEFAULT_SHADER_LOCATION + "dxr/" + m_shaderFilename + ".hlsl", { m_rayGenName, m_closestHitName, m_missName});
	psoBuilder.addHitGroup(m_hitGroupTriangleName, m_closestHitName);

	psoBuilder.addSignatureToShaders({ m_rayGenName }, m_localSignatureRayGen->get());
	psoBuilder.addSignatureToShaders({ m_hitGroupTriangleName }, m_localSignatureHitGroup->get());
	psoBuilder.addSignatureToShaders({ m_missName }, m_localSignatureMiss->get());

	psoBuilder.addLibrary(Shader::DEFAULT_SHADER_LOCATION + "dxr/ShadowRay.hlsl", { m_shadowMissName });
	psoBuilder.addSignatureToShaders({ m_shadowMissName }, m_localSignatureEmpty->get());

	psoBuilder.setMaxPayloadSize(m_settings.maxPayloadSize);
	psoBuilder.setMaxAttributeSize(m_settings.maxAttributeSize);
	psoBuilder.setMaxRecursionDepth(m_settings.maxRecursionDepth);
	psoBuilder.setGlobalSignature(m_dxrGlobalRootSignature->get());

	m_pipelineState = psoBuilder.build(context->getDevice());
}

void DXRBase::createDXRGlobalRootSignature() {
	m_dxrGlobalRootSignature = std::make_unique<DX12Utils::RootSignature>("dxrGlobal");
	m_dxrGlobalRootSignature->addSRV("AccelerationStructure", 0);
	// Add constants defined in derived class
	for (auto& it : globalConstants) {
		m_dxrGlobalRootSignature->addCBV(it.first, it.second.shaderRegister, it.second.space);
	}

	m_dxrGlobalRootSignature->build(context->getDevice());
}

void DXRBase::createRayGenLocalRootSignature() {
	m_localSignatureRayGen = std::make_unique<DX12Utils::RootSignature>("RayGenLocal");
	// Add descriptor tables defined in derived class
	for (auto& it : rayGenDescriptorTables) {
		m_localSignatureRayGen->addDescriptorTable(it.first, it.second.type, it.second.shaderRegister, it.second.space, it.second.numDescriptors);
	}
	m_localSignatureRayGen->addDescriptorTable("OutputUAVs", D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 0, m_settings.numOutputTextures); // Uav (u0, .., un) space 0 where n = numOutputTextures
	m_localSignatureRayGen->addStaticSampler();

	m_localSignatureRayGen->build(context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createHitGroupLocalRootSignature() {
	m_localSignatureHitGroup = std::make_unique<DX12Utils::RootSignature>("HitGroupLocal");
	
	if (m_settings.bindVertexBuffers) {
		m_localSignatureHitGroup->addSRV("IndexBuffer", 3, 0);
		m_localSignatureHitGroup->addSRV("PositionsBuffer", 3, 1);
		m_localSignatureHitGroup->addSRV("TexCoordsBuffer", 3, 2);
		m_localSignatureHitGroup->addSRV("NormalsBuffer", 3, 3);
		m_localSignatureHitGroup->addSRV("TangentsBuffer", 3, 4);
		m_localSignatureHitGroup->addSRV("BitangentsBuffer", 3, 5);
	}
	if (m_settings.bindTextures) {
		m_localSignatureHitGroup->addDescriptorTable("TexturesAlbedo", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 3); // Textures (t4, t5, t6) space0
		m_localSignatureHitGroup->addDescriptorTable("TexturesNormal", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 1, 3); // Textures (t4, t5, t6) space1
		m_localSignatureHitGroup->addDescriptorTable("TexturesMRAO", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 2, 3); // Textures (t4, t5, t6) space2
	}
	if (m_settings.bindVertexBuffers || m_settings.bindTextures)
		m_localSignatureHitGroup->addSRV("MeshData", 0, 1);
	m_localSignatureHitGroup->addStaticSampler();

	m_localSignatureHitGroup->build(context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createMissLocalRootSignature() {
	m_localSignatureMiss = std::make_unique<DX12Utils::RootSignature>("MissLocal");
	m_localSignatureMiss->build(context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createEmptyLocalRootSignature() {
	m_localSignatureEmpty = std::make_unique<DX12Utils::RootSignature>("EmptyLocal");
	m_localSignatureEmpty->build(context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::recreateResources() {
	createInitialShaderResources(true);
}

D3D12_GPU_VIRTUAL_ADDRESS DXRBase::GetTLASAddress() {
	auto frameIndex = Application::getInstance()->getAPI<DX12API>()->getSwapIndex();
	return m_topBuffer[frameIndex].resultGpuAddress;
}
