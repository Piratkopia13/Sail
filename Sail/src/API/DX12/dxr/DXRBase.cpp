#include "pch.h"
#include "DXRBase.h"
#include "Sail/Application.h"
#include "API/DX12/DX12VertexBuffer.h"
#include "API/DX12/DX12IndexBuffer.h"
#include "API/DX12/resources/DX12Texture.h"
#include "Sail/graphics/light/LightSetup.h"

// Include defines shared with dxr shaders
#include "Sail/../../Demo/res/shaders/dxr/dxr.shared"


DXRBase::DXRBase(const std::string& shaderFilename)
	: m_shaderFilename(shaderFilename)
{

	m_context = Application::getInstance()->getAPI<DX12API>();

	// Create frame resources (one per swap buffer)
	// Only one TLAS is used for the whole scene
	m_topBuffer = m_context->createFrameResource<AccelerationStructureBuffers>();
	m_bottomBuffers = m_context->createFrameResource<std::unordered_map<Mesh*, InstanceList>>();
	m_rayGenShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();
	m_missShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();
	m_hitGroupShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();

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
	for (auto& tlas : m_topBuffer) {
		tlas.release();
	}
	for (auto& st : m_rayGenShaderTable) {
		st.release();
	}
	for (auto& st : m_missShaderTable) {
		st.release();
	}
	for (auto& st : m_hitGroupShaderTable) {
		st.release();
	}
}

void DXRBase::updateAccelerationStructures(const std::vector<Renderer::RenderCommand>& sceneGeometry, ID3D12GraphicsCommandList4* cmdList) {

	unsigned int frameIndex = m_context->getSwapIndex();
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
					// Mesh already has a BLAS - add transform to instance list
					searchResult->second.instanceList.emplace_back((glm::mat3x4)renderCommand.transform);
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
				searchResult->second.instanceList.emplace_back((glm::mat3x4)renderCommand.transform);
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

	createTLAS(totalNumInstances, cmdList);
	updateDescriptorHeap(cmdList);
	updateShaderTables();
}

void DXRBase::updateSceneData(Camera* cam, LightSetup* lights) {
	DXRShaderCommon::SceneCBuffer newData = {};
	if (cam) {
		newData.cameraPosition = cam->getPosition();
		newData.projectionToWorld = glm::inverse(cam->getViewProjection());
	}

	/*if (lights) {
		auto& plData = lights->getPointLightsData();
		memcpy(newData.pointLights, plData.pLights, sizeof(plData));
	}*/

	m_sceneCB->updateData(&newData, sizeof(newData), 0U);
}

void DXRBase::dispatch(DX12RenderableTexture* outputTexture, ID3D12GraphicsCommandList4* cmdList) {
	unsigned int frameIndex = m_context->getSwapIndex();

	auto copyDescriptor = [&](DX12RenderableTexture* texture, D3D12_CPU_DESCRIPTOR_HANDLE* cdh) {
		// Copy output texture uav to heap
		//texture->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE); // This transition is done in RaytracingRenderer::runShading()
		m_context->getDevice()->CopyDescriptorsSimple(1, cdh[frameIndex], texture->getUavCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	};
	copyDescriptor(outputTexture, m_outputResource.cpuHandle);
	
	// Set constant buffer descriptor heap
	m_descriptorHeap->bind(cmdList);

	// Let's raytrace

	D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
	raytraceDesc.Width = Application::getInstance()->getWindow()->getWindowWidth();
	raytraceDesc.Height = Application::getInstance()->getWindow()->getWindowHeight();
	raytraceDesc.Depth = 1;

	//set shader tables
	raytraceDesc.RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable[frameIndex].Resource->GetGPUVirtualAddress();
	raytraceDesc.RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable[frameIndex].SizeInBytes;

	raytraceDesc.MissShaderTable.StartAddress = m_missShaderTable[frameIndex].Resource->GetGPUVirtualAddress();
	raytraceDesc.MissShaderTable.StrideInBytes = m_missShaderTable[frameIndex].StrideInBytes;
	raytraceDesc.MissShaderTable.SizeInBytes = m_missShaderTable[frameIndex].SizeInBytes;

	raytraceDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable[frameIndex].Resource->GetGPUVirtualAddress();
	raytraceDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTable[frameIndex].StrideInBytes;
	raytraceDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable[frameIndex].SizeInBytes;

	// Bind the global root signature
	cmdList->SetComputeRootSignature(*m_dxrGlobalRootSignature->get());

	// Set acceleration structure
	cmdList->SetComputeRootShaderResourceView(m_dxrGlobalRootSignature->getIndex("AccelerationStructure"), m_topBuffer[frameIndex].result->GetGPUVirtualAddress());
	// Set scene constant buffer
	cmdList->SetComputeRootConstantBufferView(m_dxrGlobalRootSignature->getIndex("SceneCBuffer"), m_sceneCB->getBuffer()->GetGPUVirtualAddress());

	// Dispatch
	cmdList->SetPipelineState1(m_pipelineState.Get());
	cmdList->DispatchRays(&raytraceDesc);
}

//void DXRBase::reloadShaders() {
//	m_context->waitForGPU();
//	// Recompile hlsl
//	createRaytracingPSO();
//}

void DXRBase::createTLAS(unsigned int numInstanceDescriptors, ID3D12GraphicsCommandList4* cmdList) {

	// Always rebuilds TLAS instead of updating it according to nvidia recommendations

	unsigned int frameIndex = m_context->getSwapIndex();

	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = numInstanceDescriptors;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	m_topBuffer[frameIndex].release();

	// Re-create the buffer
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	m_context->getDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	// Create the buffers
	if (m_topBuffer[frameIndex].scratch == nullptr) {
		m_topBuffer[frameIndex].scratch = DX12Utils::CreateBuffer(m_context->getDevice(), info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::sDefaultHeapProps);
		m_topBuffer[frameIndex].scratch->SetName(L"TLAS_SCRATCH");
	}

	if (m_topBuffer[frameIndex].result == nullptr) {
		m_topBuffer[frameIndex].result = DX12Utils::CreateBuffer(m_context->getDevice(), info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, DX12Utils::sDefaultHeapProps);
		m_topBuffer[frameIndex].result->SetName(L"TLAS_RESULT");
	}

	m_topBuffer[frameIndex].instanceDesc = DX12Utils::CreateBuffer(m_context->getDevice(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * glm::max(numInstanceDescriptors, 1U), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties);
	m_topBuffer[frameIndex].instanceDesc->SetName(L"TLAS_INSTANCE_DESC");

	D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc;
	m_topBuffer[frameIndex].instanceDesc->Map(0, nullptr, (void**)& pInstanceDesc);

	unsigned int blasIndex = 0;
	unsigned int instanceID = 0;

	for (auto& it : m_bottomBuffers[frameIndex]) {
		auto& instanceList = it.second;
		for (auto& transform : instanceList.instanceList) {
			pInstanceDesc->InstanceID = blasIndex;
			pInstanceDesc->InstanceMask = 0xFF;
			
			pInstanceDesc->InstanceContributionToHitGroupIndex = blasIndex * 2;	// offset inside the shader-table. Unique for every instance since each geometry has different vertexbuffer/indexbuffer/textures
																				// * 2 since every other entry in the SBT is for shadow rays (NULL hit group)
			pInstanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

			memcpy(pInstanceDesc->Transform, &transform, sizeof(pInstanceDesc->Transform));

			pInstanceDesc->AccelerationStructure = instanceList.blas.result->GetGPUVirtualAddress();

			pInstanceDesc++;
		}
		blasIndex++;
	}

	// Unmap
	m_topBuffer[frameIndex].instanceDesc->Unmap(0, nullptr);

	// Create the TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = m_topBuffer[frameIndex].instanceDesc->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = m_topBuffer[frameIndex].result->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = m_topBuffer[frameIndex].scratch->GetGPUVirtualAddress();

	cmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// UAV barrier needed before using the acceleration structures in a raytracing operation
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = m_topBuffer[frameIndex].result.Get();
	cmdList->ResourceBarrier(1, &uavBarrier);
}

void DXRBase::createBLAS(const Renderer::RenderCommand& renderCommand, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags, ID3D12GraphicsCommandList4* cmdList, AccelerationStructureBuffers* sourceBufferForUpdate) {
	unsigned int frameIndex = m_context->getSwapIndex();
	Mesh* mesh = renderCommand.mesh;

	bool performInplaceUpdate = (sourceBufferForUpdate) ? true : false;

	InstanceList instance;
	instance.instanceList.emplace_back(renderCommand.transform);
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
	geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Mesh::vec3); // TODO: fix, as this is only for the positions
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
	m_context->getDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	// TODO: make sure buffer size is >= info.UpdateScratchDataSize in bytes
	if (!performInplaceUpdate) {
		// Create the buffers. They need to support UAV, and since we are going to immediately use them, we create them with an unordered-access state
		bottomBuffer.scratch = DX12Utils::CreateBuffer(m_context->getDevice(), info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::sDefaultHeapProps);
		bottomBuffer.scratch->SetName(L"BLAS_SCRATCH");
		bottomBuffer.result = DX12Utils::CreateBuffer(m_context->getDevice(), info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, DX12Utils::sDefaultHeapProps);
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
		// Insert BLAS into buttom buffer map
		m_bottomBuffers[frameIndex].insert({ mesh, instance });
	}
}

void DXRBase::createInitialShaderResources(bool remake) {
	// Create some resources only once on init
	if (!m_descriptorHeap || remake) {
		UINT numDescriptors = 5000;
		m_descriptorHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescriptors, true, true);

		// The first 10 slots in the heap will be used for the output UAVs and history input SRV
		auto storeHandle = [&](Resource& res) {
			for (unsigned int i = 0; i < 2; i++) {
				auto index = m_descriptorHeap->getAndStepIndex();
				res.cpuHandle[i] = m_descriptorHeap->getCPUDescriptorHandleForIndex(index);
				res.gpuHandle[i] = m_descriptorHeap->getGPUDescriptorHandleForIndex(index);
			}
		};
		// UAV output
		storeHandle(m_outputResource);

		// Scene CB
		{
			unsigned int size = sizeof(DXRShaderCommon::SceneCBuffer);
			DXRShaderCommon::SceneCBuffer initData = {};
			m_sceneCB = std::make_unique<ShaderComponent::DX12ConstantBuffer>(&initData, size, ShaderComponent::BIND_SHADER::CS, 0);
		}
		//// Mesh CB
		//{
		//	unsigned int size = sizeof(DXRShaderCommon::MeshCBuffer);
		//	void* initData = malloc(size);
		//	memset(initData, 0, size);
		//	m_meshCB = std::make_unique<ShaderComponent::DX12ConstantBuffer>(initData, size, ShaderComponent::BIND_SHADER::CS, 0);
		//	free(initData);
		//}
	}
}

void DXRBase::updateDescriptorHeap(ID3D12GraphicsCommandList4* cmdList) {
	unsigned int frameIndex = m_context->getSwapIndex();
		
}

void DXRBase::updateShaderTables() {

	// 	 "Shader tables can be modified freely by the application (with appropriate state barriers)"

	auto frameIndex = m_context->getSwapIndex();

	// Ray gen
	{
		if (m_rayGenShaderTable[frameIndex].Resource) {
			m_rayGenShaderTable[frameIndex].Resource->Release();
			m_rayGenShaderTable[frameIndex].Resource.Reset();
		}
		DXRUtils::ShaderTableBuilder tableBuilder(1U, m_pipelineState.Get(), 96U);
		tableBuilder.addShader(m_rayGenName);
		tableBuilder.addDescriptor(m_outputResource.gpuHandle[frameIndex].ptr);
		m_rayGenShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
	}

	// Miss
	{
		if (m_missShaderTable[frameIndex].Resource) {
			m_missShaderTable[frameIndex].Resource->Release();
			m_missShaderTable[frameIndex].Resource.Reset();
		}

		DXRUtils::ShaderTableBuilder tableBuilder(2U, m_pipelineState.Get());
		tableBuilder.addShader(m_missName);
		tableBuilder.addShader(m_shadowMissName);
		m_missShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
	}

	// Hit group
	{
		if (m_hitGroupShaderTable[frameIndex].Resource) {
			m_hitGroupShaderTable[frameIndex].Resource->Release();
			m_hitGroupShaderTable[frameIndex].Resource.Reset();
		}

		UINT numInstances = (UINT)m_bottomBuffers[frameIndex].size() * 2U; // * 2 for shadow rays (all NULL)
		DXRUtils::ShaderTableBuilder tableBuilder(numInstances, m_pipelineState.Get(), 64U);

		unsigned int blasIndex = 0;

		for (auto& it : m_bottomBuffers[frameIndex]) {
			auto& instanceList = it.second;
			Mesh* mesh = it.first;

			tableBuilder.addShader(m_hitGroupTriangleName); //Set the shadergroup to use
			m_localSignatureHitGroup->doInOrder([&](const std::string& parameterName) {
				Logger::Error("Unhandled root signature parameter! (" + parameterName + ")");
			});
			
			tableBuilder.addShader(L"NULL");
			blasIndex++;
		}

		m_hitGroupShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
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

	psoBuilder.setMaxPayloadSize(sizeof(DXRShaderCommon::RayPayload));
	psoBuilder.setMaxAttributeSize(sizeof(float) * 4);
	psoBuilder.setMaxRecursionDepth(DXRShaderCommon::MAX_RAY_RECURSION_DEPTH);
	psoBuilder.setGlobalSignature(m_dxrGlobalRootSignature->get());

	m_pipelineState = psoBuilder.build(m_context->getDevice());
}

void DXRBase::createDXRGlobalRootSignature() {
	m_dxrGlobalRootSignature = std::make_unique<DX12Utils::RootSignature>("dxrGlobal");
	m_dxrGlobalRootSignature->addSRV("AccelerationStructure", 0);
	m_dxrGlobalRootSignature->addCBV("SceneCBuffer", 0);

	m_dxrGlobalRootSignature->build(m_context->getDevice());
}

void DXRBase::createRayGenLocalRootSignature() {
	m_localSignatureRayGen = std::make_unique<DX12Utils::RootSignature>("RayGenLocal");
	m_localSignatureRayGen->addDescriptorTable("OutputUAV", D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);
	m_localSignatureRayGen->addStaticSampler();

	// Border sampler used to sample InputShadowsLastFrame
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.MipLODBias = 0.f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.ShaderRegister = 1;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	m_localSignatureRayGen->addStaticSampler(samplerDesc);

	m_localSignatureRayGen->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createHitGroupLocalRootSignature() {
	m_localSignatureHitGroup = std::make_unique<DX12Utils::RootSignature>("HitGroupLocal");
	m_localSignatureHitGroup->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createMissLocalRootSignature() {
	m_localSignatureMiss = std::make_unique<DX12Utils::RootSignature>("MissLocal");
	m_localSignatureMiss->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createEmptyLocalRootSignature() {
	m_localSignatureEmpty = std::make_unique<DX12Utils::RootSignature>("EmptyLocal");
	m_localSignatureEmpty->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}
