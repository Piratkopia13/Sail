#include "pch.h"
#include "DXRBase.h"
#include "Sail/Application.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "API/DX12/DX12VertexBuffer.h"
#include "API/DX12/DX12IndexBuffer.h"
#include "API/DX12/resources/DX12Texture.h"
#include "Sail/graphics/light/LightSetup.h"
#include "../renderer/DX12GBufferRenderer.h"

DXRBase::DXRBase(const std::string& shaderFilename, DX12RenderableTexture** inputs)
: m_shaderFilename(shaderFilename)
, m_gbufferInputTextures(inputs)
, m_brdfLUTPath("pbr/brdfLUT.tga")
{
	m_context = Application::getInstance()->getAPI<DX12API>();

	// Create frame resources (one per swap buffer)
	// Only one TLAS is used for the whole scene
	m_DXR_TopBuffer = m_context->createFrameResource<AccelerationStructureBuffers>();
	m_bottomBuffers = m_context->createFrameResource<std::unordered_map<Mesh*, InstanceList>>();
	m_rayGenShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();
	m_missShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();
	m_hitGroupShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();
	m_gbufferStartGPUHandles = m_context->createFrameResource<D3D12_GPU_DESCRIPTOR_HANDLE>();

	// Create root signatures
	createDXRGlobalRootSignature();
	createRayGenLocalRootSignature();
	createHitGroupLocalRootSignature();
	createMissLocalRootSignature();
	createEmptyLocalRootSignature();

	createRaytracingPSO();
	createInitialShaderResources();

	m_aabb_desc_resource = DX12Utils::CreateBuffer(m_context->getDevice(), sizeof(m_aabb_desc), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties);
	m_aabb_desc_resource->SetName(L"AABB Data");
	void* pMappedData;
	m_aabb_desc_resource->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, &m_aabb_desc, sizeof(m_aabb_desc));
	m_aabb_desc_resource->Unmap(0, nullptr);

}

DXRBase::~DXRBase() {
	m_rtPipelineState->Release();
	for (auto& blasList : m_bottomBuffers) {
		for (auto& blas : blasList) {
			blas.second.blas.release();
		}
	}
	for (auto& tlas : m_DXR_TopBuffer) {
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

	for (auto& resource : m_metaballPositions_srv) {
		resource->Release();
	}

	m_aabb_desc_resource->Release();
}

void DXRBase::setGBufferInputs(DX12RenderableTexture** inputs) {
	m_gbufferInputTextures = inputs;
}

void DXRBase::updateAccelerationStructures(const std::vector<Renderer::RenderCommand>& sceneGeometry, ID3D12GraphicsCommandList4* cmdList) {

	unsigned int frameIndex = m_context->getFrameIndex();
	unsigned int totalNumInstances = 0;

	auto flagNone = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	auto flagFastTrace = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	auto flagFastBuild = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	auto flagAllowUpdate = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	// Clear old instance lists
	for (auto& it : m_bottomBuffers[frameIndex]) {
		it.second.instanceTransforms.clear();
	}

	// Iterate all static meshes
	for (auto& renderCommand : sceneGeometry) {
		if (renderCommand.flags & Renderer::MESH_STATIC) {
			Mesh* mesh = nullptr;
			if (renderCommand.type == Renderer::RENDER_COMMAND_TYPE_MODEL) {
				mesh = renderCommand.model.mesh;
			}

			auto& searchResult = m_bottomBuffers[frameIndex].find(mesh);
			// If mesh does not have a BLAS
			if (searchResult == m_bottomBuffers[frameIndex].end()) {
				createBLAS(renderCommand, flagFastTrace, cmdList);
			} else {
				if (renderCommand.hasUpdatedSinceLastRender[frameIndex]) {
					Logger::Warning("A BLAS rebuild has been triggered on a STATIC mesh. Consider changing it to DYNAMIC!");
					// Destroy old blas
					searchResult->second.blas.release();
					m_bottomBuffers[frameIndex].erase(searchResult);
					// Create new one
					createBLAS(renderCommand, flagFastTrace, cmdList);
				} else {
					// Mesh already has a BLAS - add transform to instance list
					searchResult->second.instanceTransforms.emplace_back(renderCommand.transform);
				}
			}

			totalNumInstances++;
		}
	}

	// Iterate all dynamic meshes
	for (auto& renderCommand : sceneGeometry) {
		if (renderCommand.flags & Renderer::MESH_DYNAMIC) {
			Mesh* mesh = nullptr;
			if (renderCommand.type == Renderer::RENDER_COMMAND_TYPE_MODEL) {
				mesh = renderCommand.model.mesh;
			}

			auto& searchResult = m_bottomBuffers[frameIndex].find(mesh);
			auto flags = flagNone;
			if (renderCommand.flags & Renderer::MESH_HERO) {
				flags = flagFastTrace | flagAllowUpdate;
			} else {
				flags = flagFastBuild | flagAllowUpdate;
			}

			// If mesh does not have a BLAS or was first built as STATIC
			if (searchResult == m_bottomBuffers[frameIndex].end() || !searchResult->second.blas.allowUpdate) {
				createBLAS(renderCommand, flags, cmdList);
			} else {
				if (renderCommand.hasUpdatedSinceLastRender[frameIndex]) {
					createBLAS(renderCommand, flags, cmdList, &searchResult->second.blas);
				}
				// Add transform to instance list
				searchResult->second.instanceTransforms.emplace_back(renderCommand.transform);
			}

			totalNumInstances++;
		}
	}

	// Destroy BLASes that are no longer part of the scene
	for (auto it = m_bottomBuffers[frameIndex].begin(); it != m_bottomBuffers[frameIndex].end();) {
		bool destroy = true;
		for (auto& renderCommand : sceneGeometry) {
			Mesh* mesh = nullptr;
			if (renderCommand.type == Renderer::RENDER_COMMAND_TYPE_MODEL) {
				mesh = renderCommand.model.mesh;
			}

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

void DXRBase::updateSceneData(Camera& cam, LightSetup& lights, const std::vector<Metaball>& metaballs) {
	m_metaballsToRender = (metaballs.size() < MAX_NUM_METABALLS) ? (UINT)metaballs.size() : (UINT)MAX_NUM_METABALLS;
	updateMetaballpositions(metaballs);

	DXRShaderCommon::SceneCBuffer newData = {};
	newData.viewToWorld = glm::inverse(cam.getViewMatrix());
	newData.clipToView = glm::inverse(cam.getProjMatrix());
	newData.nearZ = cam.getNearZ();
	newData.farZ = cam.getFarZ();
	newData.cameraPosition = cam.getPosition();
	newData.projectionToWorld = glm::inverse(cam.getViewProjection());
	newData.nMetaballs = m_metaballsToRender;

	auto& plData = lights.getPointLightsData();
	memcpy(newData.pointLights, plData.pLights, sizeof(plData));
	m_sceneCB->updateData(&newData, sizeof(newData));
}

void DXRBase::updateMetaballpositions(const std::vector<Metaball>& metaballs) {
	if (m_metaballsToRender == 0) {
		return;
	}

	void* pMappedData;
	ID3D12Resource1* res = m_metaballPositions_srv[m_context->getFrameIndex()];

 	HRESULT hr =  res->Map(0, nullptr, &pMappedData);
	if (FAILED(hr)) {
		_com_error err(hr);
		std::cout << err.ErrorMessage() << std::endl;

		hr = m_context->getDevice()->GetDeviceRemovedReason();
		_com_error err2(hr);
		std::cout << err2.ErrorMessage() << std::endl;

		return;
	}
	int offset = 0;
	int offsetInc = sizeof(metaballs[0].pos);
	for (size_t i = 0; i < m_metaballsToRender; i++) {;
		memcpy(static_cast<char*>(pMappedData) + offset, &metaballs[i].pos, offsetInc);
		offset += offsetInc;
	}
	res->Unmap(0, nullptr);
}

void DXRBase::dispatch(DX12RenderableTexture* outputTexture, ID3D12GraphicsCommandList4* cmdList) {

	assert(m_gbufferInputTextures); // Input textures not set!

	for (int i = 0; i < DX12GBufferRenderer::NUM_GBUFFERS; i++) {
		m_gbufferInputTextures[i]->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	
	unsigned int frameIndex = m_context->getFrameIndex();

	outputTexture->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	// Copy output texture srv to beginning of heap
	m_context->getDevice()->CopyDescriptorsSimple(1, m_rtDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), outputTexture->getUavCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_rtDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);


	// Let's raytrace
	outputTexture->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//DX12Utils::SetResourceTransitionBarrier(cmdList, m_rtOutputUAV.resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

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
	cmdList->SetComputeRootShaderResourceView(m_dxrGlobalRootSignature->getIndex("AccelerationStructure"), m_DXR_TopBuffer[frameIndex].result->GetGPUVirtualAddress());
	// Set scene constant buffer
	cmdList->SetComputeRootConstantBufferView(m_dxrGlobalRootSignature->getIndex("SceneCBuffer"), m_sceneCB->getBuffer()->GetGPUVirtualAddress());

	// Dispatch
	cmdList->SetPipelineState1(m_rtPipelineState.Get());
	cmdList->DispatchRays(&raytraceDesc);
}

void DXRBase::reloadShaders() {
	m_context->waitForGPU();
	// Recompile hlsl
	createRaytracingPSO();
}

bool DXRBase::onEvent(Event& event) {
	auto onResize = [&](WindowResizeEvent& event) {
		// Window changed size, resize output UAV
		createInitialShaderResources(true);
		return true;
	};

	EventHandler::dispatch<WindowResizeEvent>(event, onResize);
	return true;
}

void DXRBase::createTLAS(unsigned int numInstanceDescriptors, ID3D12GraphicsCommandList4* cmdList) {

	// Always rebuilds TLAS instead of updating it according to nvidia recommendations

	unsigned int frameIndex = m_context->getFrameIndex();

	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = numInstanceDescriptors;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	m_DXR_TopBuffer[frameIndex].release();

	// Re-create the buffer
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	m_context->getDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	// Create the buffers
	if (m_DXR_TopBuffer[frameIndex].scratch == nullptr) {
		m_DXR_TopBuffer[frameIndex].scratch = DX12Utils::CreateBuffer(m_context->getDevice(), info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::sDefaultHeapProps);
		m_DXR_TopBuffer[frameIndex].scratch->SetName(L"TLAS_SCRATCH");
	}

	if (m_DXR_TopBuffer[frameIndex].result == nullptr) {
		m_DXR_TopBuffer[frameIndex].result = DX12Utils::CreateBuffer(m_context->getDevice(), info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, DX12Utils::sDefaultHeapProps);
		m_DXR_TopBuffer[frameIndex].result->SetName(L"TLAS_RESULT");
	}

	m_DXR_TopBuffer[frameIndex].instanceDesc = DX12Utils::CreateBuffer(m_context->getDevice(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * glm::max(numInstanceDescriptors, 1U), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties);
	m_DXR_TopBuffer[frameIndex].instanceDesc->SetName(L"TLAS_INSTANCE_DESC");

	D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc;
	m_DXR_TopBuffer[frameIndex].instanceDesc->Map(0, nullptr, (void**)& pInstanceDesc);

	unsigned int blasIndex = 0;
	unsigned int instanceID = 0;
	unsigned int instanceID_metaballs = 0;
	for (auto& it : m_bottomBuffers[frameIndex]) {
		auto& instanceList = it.second;
		for (auto& transform : instanceList.instanceTransforms) {
			if (it.first == nullptr) {
				pInstanceDesc->InstanceID = instanceID_metaballs++;	// exposed to the shader via InstanceID() - currently same for all instances of same material
				pInstanceDesc->InstanceMask = 0x01;
			} else {
				pInstanceDesc->InstanceID = blasIndex;
				pInstanceDesc->InstanceMask = 0xFF &~ 0x01;
			}
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
	m_DXR_TopBuffer[frameIndex].instanceDesc->Unmap(0, nullptr);

	// Create the TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = m_DXR_TopBuffer[frameIndex].instanceDesc->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = m_DXR_TopBuffer[frameIndex].result->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = m_DXR_TopBuffer[frameIndex].scratch->GetGPUVirtualAddress();

	cmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// UAV barrier needed before using the acceleration structures in a raytracing operation
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = m_DXR_TopBuffer[frameIndex].result.Get();
	cmdList->ResourceBarrier(1, &uavBarrier);
}

void DXRBase::createBLAS(const Renderer::RenderCommand& renderCommand, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags, ID3D12GraphicsCommandList4* cmdList, AccelerationStructureBuffers* sourceBufferForUpdate) {
	unsigned int frameIndex = m_context->getFrameIndex();
	Mesh* mesh = nullptr; 
	if (renderCommand.type == Renderer::RENDER_COMMAND_TYPE_MODEL) {
		mesh = renderCommand.model.mesh;
	}

	bool performInplaceUpdate = (sourceBufferForUpdate) ? true : false;

	InstanceList instance;
	instance.instanceTransforms.emplace_back(renderCommand.transform);
	AccelerationStructureBuffers& bottomBuffer = instance.blas;
	if (performInplaceUpdate) {
		bottomBuffer = *sourceBufferForUpdate;
	}
	if (flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) {
		instance.blas.allowUpdate = true;
	}

	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	if (renderCommand.type == Renderer::RENDER_COMMAND_TYPE_MODEL) {
		auto& vb = static_cast<DX12VertexBuffer&>(mesh->getVertexBuffer());
		auto& ib = static_cast<const DX12IndexBuffer&>(mesh->getIndexBuffer());

		// Make sure vbuffer is initialized
		vb.init(cmdList);

		geomDesc.Flags = (renderCommand.flags & Renderer::MESH_TRANSPARENT) ? D3D12_RAYTRACING_GEOMETRY_FLAG_NONE : D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geomDesc.Triangles.VertexBuffer.StartAddress = vb.getBuffer()->GetGPUVirtualAddress();
		geomDesc.Triangles.VertexBuffer.StrideInBytes = vb.getVertexDataStride();
		geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geomDesc.Triangles.VertexCount = mesh->getNumVertices();

		if (mesh->getNumIndices() > 0) {
			geomDesc.Triangles.IndexBuffer = ib.getBuffer()->GetGPUVirtualAddress();
			geomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
			geomDesc.Triangles.IndexCount = UINT(mesh->getNumIndices());
		}
	} else {
		//No mesh included. Use AABB from GPU memmory (m_aabb_desc_resource) and set type to PROCEDURAL_PRIMITIVE.
		geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
		geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		geomDesc.AABBs.AABBCount = 1;
		geomDesc.AABBs.AABBs.StartAddress = m_aabb_desc_resource->GetGPUVirtualAddress();
		geomDesc.AABBs.AABBs.StrideInBytes = 0;
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
	initMetaballBuffers();

	// Create some resources only once on init
	if (!m_rtDescriptorHeap || remake) {
		m_rtDescriptorHeap.Reset();
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 2000; // TODO: this does not throw error when full
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_context->getDevice()->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&m_rtDescriptorHeap));

		m_heapIncr = m_context->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Create the UAV. Based on the root signature we created it should be the first entry
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_rtDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_rtDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		// The first slot in the heap will be used for the output UAV, therefore we step once
		m_rtOutputTextureUavGPUHandle = gpuHandle;
		cpuHandle.ptr += m_heapIncr;
		gpuHandle.ptr += m_heapIncr;

		// Next slot is used for the brdfLUT
		m_rtBrdfLUTGPUHandle = gpuHandle;
		if (!Application::getInstance()->getResourceManager().hasTexture(m_brdfLUTPath)) {
			Application::getInstance()->getResourceManager().loadTexture(m_brdfLUTPath);
		}
		auto& brdfLutTex = static_cast<DX12Texture&>(Application::getInstance()->getResourceManager().getTexture(m_brdfLUTPath));
		m_context->getDevice()->CopyDescriptorsSimple(1, cpuHandle, brdfLutTex.getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		cpuHandle.ptr += m_heapIncr;
		gpuHandle.ptr += m_heapIncr;

		// Next (4 * numSwapBuffers) slots are used for input gbuffers
		for (unsigned int i = 0; i < m_context->getNumSwapBuffers(); i++) {
			m_gbufferStartGPUHandles[i] = gpuHandle;
			D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors[4];
			srcDescriptors[0] = m_gbufferInputTextures[0]->getSrvCDH(i);
			srcDescriptors[1] = m_gbufferInputTextures[1]->getSrvCDH(i);
			srcDescriptors[2] = m_gbufferInputTextures[2]->getSrvCDH(i);
			srcDescriptors[3] = m_gbufferInputTextures[0]->getDepthSrvCDH(i);

			UINT dstRangeSizes[] = { 4 };
			UINT srcRangeSizes[] = { 1, 1, 1, 1 };
			m_context->getDevice()->CopyDescriptors(1, &cpuHandle, dstRangeSizes, 4, srcDescriptors, srcRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			cpuHandle.ptr += m_heapIncr * 4;
			gpuHandle.ptr += m_heapIncr * 4;
		}

		//// Ray gen settings CB
		//m_rayGenCBData.flags = RT_ENABLE_TA | RT_ENABLE_JITTER_AA;
		//m_rayGenCBData.numAORays = 5;
		//m_rayGenCBData.AORadius = 0.9f;
		//m_rayGenCBData.frameCount = 0;
		//m_rayGenCBData.GISamples = 1;
		//m_rayGenCBData.GIBounces = 1;
		//m_rayGenSettingsCB = std::make_unique<DX12ConstantBuffer>("Ray Gen Settings CB", sizeof(RayGenSettings), m_renderer);
		//m_rayGenSettingsCB->setData(&m_rayGenCBData, 0);

		// Store heap start for views that might update in runtime
		m_rtHeapCPUHandle = cpuHandle;
		m_rtHeapGPUHandle = gpuHandle;

		// Scene CB
		{
			unsigned int size = sizeof(DXRShaderCommon::SceneCBuffer);
			void* initData = malloc(size);
			memset(initData, 0, size);
			m_sceneCB = std::make_unique<ShaderComponent::DX12ConstantBuffer>(initData, size, ShaderComponent::BIND_SHADER::CS, 0);
			free(initData);
		}
		// Mesh CB
		{
			unsigned int size = sizeof(DXRShaderCommon::MeshCBuffer);
			void* initData = malloc(size);
			memset(initData, 0, size);
			m_meshCB = std::make_unique<ShaderComponent::DX12ConstantBuffer>(initData, size, ShaderComponent::BIND_SHADER::CS, 0);
			free(initData);
		}
	}

}

void DXRBase::updateDescriptorHeap(ID3D12GraphicsCommandList4* cmdList) {
	unsigned int frameIndex = m_context->getFrameIndex();

	// Make sure brdfLut texture has been initialized
	auto& brdfLutTex = static_cast<DX12Texture&>(Application::getInstance()->getResourceManager().getTexture(m_brdfLUTPath));
	if (!brdfLutTex.hasBeenInitialized()) {
		brdfLutTex.initBuffers(cmdList, 0);
	}

	// Update descriptors for vertices, indices, textures etc
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_rtHeapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_rtHeapGPUHandle;
	m_rtMeshHandles.clear();

	unsigned int blasIndex = 0;
	unsigned int metaballIndex = 0;
	for (auto& it : m_bottomBuffers[frameIndex]) {
		auto& instanceList = it.second;
		Mesh* mesh = it.first;

		unsigned int meshDataSize = sizeof(DXRShaderCommon::MeshData);
		DXRShaderCommon::MeshData meshData;
		
		MeshHandles handles;
		if (mesh) {
			handles.vertexBufferHandle = static_cast<const DX12VertexBuffer&>(mesh->getVertexBuffer()).getBuffer()->GetGPUVirtualAddress();
			if (mesh->getNumIndices() > 0) {
				handles.indexBufferHandle = static_cast<const DX12IndexBuffer&>(mesh->getIndexBuffer()).getBuffer()->GetGPUVirtualAddress();
			}

			auto& materialSettings = mesh->getMaterial()->getPBRSettings();

			// Three textures
			for (unsigned int textureNum = 0; textureNum < 3; textureNum++) {
				DX12Texture* texture = static_cast<DX12Texture*>(mesh->getMaterial()->getTexture(textureNum));
				bool hasTexture = (textureNum == 0) ? materialSettings.hasAlbedoTexture : materialSettings.hasNormalTexture;
				hasTexture = (textureNum == 2) ? materialSettings.hasMetalnessRoughnessAOTexture : hasTexture;
				if (hasTexture) {
					// Make sure textures have initialized / uploaded their data to its default buffer
					if (!texture->hasBeenInitialized()) {
						texture->initBuffers(cmdList, textureNum * blasIndex);
					}

						// Copy SRV to DXR heap
						m_context->getDevice()->CopyDescriptorsSimple(1, cpuHandle, texture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						handles.textureHandles[textureNum] = gpuHandle;
					}
					// Increase pointer regardless of if the texture existed or not to keep to order in the SBT
					cpuHandle.ptr += m_heapIncr;
					gpuHandle.ptr += m_heapIncr;
				}

      // Update per mesh data
      // Such as flags telling the shader to use indices, textures or not
      unsigned int meshDataSize = sizeof(DXRShaderCommon::MeshData);
      DXRShaderCommon::MeshData meshData;
      meshData.flags = (mesh->getNumIndices() == 0) ? DXRShaderCommon::MESH_NO_FLAGS : DXRShaderCommon::MESH_USE_INDICES;
      meshData.flags |= (materialSettings.hasAlbedoTexture) ? DXRShaderCommon::MESH_HAS_ALBEDO_TEX : meshData.flags;
      meshData.flags |= (materialSettings.hasNormalTexture) ? DXRShaderCommon::MESH_HAS_NORMAL_TEX : meshData.flags;
      meshData.flags |= (materialSettings.hasMetalnessRoughnessAOTexture) ? DXRShaderCommon::MESH_HAS_METALNESS_ROUGHNESS_AO_TEX : meshData.flags;
      meshData.color = materialSettings.modelColor;
      meshData.metalnessRoughnessAoScales.r = materialSettings.metalnessScale;
      meshData.metalnessRoughnessAoScales.g = materialSettings.roughnessScale;
      meshData.metalnessRoughnessAoScales.b = materialSettings.aoScale;
      m_meshCB->updateData(&meshData, meshDataSize, blasIndex * meshDataSize);

			m_rtMeshHandles.emplace_back(handles);
		} else {
			m_rtMeshHandles.emplace_back(handles);
			static float time = 0.f;
			static float totalTime = 0.f;
			static float inc = 0.001f;
			time += inc;
			totalTime += abs(inc);

			if (time > 1.f) {
				time = 1.f;
				inc *= -1.f;
			} else if(time < 0.f) {
				time = 0.f;
				inc *= -1.f;
			}

			//float r = ((int)time % 10) / 10.0f;

			meshData.flags = DXRShaderCommon::MESH_NO_FLAGS;
			meshData.color = glm::vec4((float)(metaballIndex++), 1.f - time, totalTime, 1.f);
			m_meshCB->updateData(&meshData, meshDataSize, blasIndex * meshDataSize);
		}

		blasIndex++;
	}
}

void DXRBase::updateShaderTables() {

	// 	 "Shader tables can be modified freely by the application (with appropriate state barriers)"

	auto frameIndex = m_context->getFrameIndex();

	// Ray gen
	{
		if (m_rayGenShaderTable[frameIndex].Resource) {
			m_rayGenShaderTable[frameIndex].Resource->Release();
			m_rayGenShaderTable[frameIndex].Resource.Reset();
		}
		DXRUtils::ShaderTableBuilder tableBuilder(1U, m_rtPipelineState.Get());
		tableBuilder.addShader(m_rayGenName);
		tableBuilder.addDescriptor(m_rtOutputTextureUavGPUHandle.ptr);
		tableBuilder.addDescriptor(m_gbufferStartGPUHandles[frameIndex].ptr);
		tableBuilder.addDescriptor(m_rtBrdfLUTGPUHandle.ptr);
		m_rayGenShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
	}

	// Miss
	{
		if (m_missShaderTable[frameIndex].Resource) {
			m_missShaderTable[frameIndex].Resource->Release();
			m_missShaderTable[frameIndex].Resource.Reset();
		}

		DXRUtils::ShaderTableBuilder tableBuilder(2U, m_rtPipelineState.Get());
		tableBuilder.addShader(m_missName);
		tableBuilder.addShader(m_shadowMissName);
		//tableBuilder.addDescriptor(m_skyboxGPUDescHandle.ptr);
		m_missShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
	}

	// Hit group
	{
		if (m_hitGroupShaderTable[frameIndex].Resource) {
			m_hitGroupShaderTable[frameIndex].Resource->Release();
			m_hitGroupShaderTable[frameIndex].Resource.Reset();
		}

		DXRUtils::ShaderTableBuilder tableBuilder((UINT)m_bottomBuffers[frameIndex].size() * 2U /* * 2 for shadow rays (all NULL) */, m_rtPipelineState.Get(), 64U);

		unsigned int blasIndex = 0;
		for (auto& it : m_bottomBuffers[frameIndex]) {
			auto& instanceList = it.second;
			Mesh* mesh = it.first;

			if (!mesh) {
				tableBuilder.addShader(m_hitGroupMetaBallName);//Set the shadergroup to use
				m_localSignatureHitGroup_metaball->doInOrder([&](const std::string& parameterName) {
					if (parameterName == "MeshCBuffer") {
						D3D12_GPU_VIRTUAL_ADDRESS meshCBHandle = m_meshCB->getBuffer()->GetGPUVirtualAddress();
						tableBuilder.addDescriptor(meshCBHandle, blasIndex * 2);
					}
					else if (parameterName == "MetaballPositions") {
						D3D12_GPU_VIRTUAL_ADDRESS metaballHandle = m_metaballPositions_srv[frameIndex]->GetGPUVirtualAddress();
						tableBuilder.addDescriptor(metaballHandle, blasIndex * 2);
					} else if (parameterName == "sys_brdfLUT") {
						tableBuilder.addDescriptor(m_rtBrdfLUTGPUHandle.ptr, blasIndex * 2);
					} else {
						Logger::Error("Unhandled root signature parameter! (" + parameterName + ")");
					}
					});
			} else {
				tableBuilder.addShader(m_hitGroupTriangleName);//Set the shadergroup to use
				m_localSignatureHitGroup_mesh->doInOrder([&](const std::string& parameterName) {
					if (parameterName == "VertexBuffer") {
						tableBuilder.addDescriptor(m_rtMeshHandles[blasIndex].vertexBufferHandle, blasIndex * 2);
					} else if (parameterName == "IndexBuffer") {
						D3D12_GPU_VIRTUAL_ADDRESS nullAddr = 0;
						tableBuilder.addDescriptor((mesh->getNumIndices() > 0) ? m_rtMeshHandles[blasIndex].indexBufferHandle : nullAddr, blasIndex * 2);
					} else if (parameterName == "MeshCBuffer") {
						D3D12_GPU_VIRTUAL_ADDRESS meshCBHandle = m_meshCB->getBuffer()->GetGPUVirtualAddress();
						tableBuilder.addDescriptor(meshCBHandle, blasIndex * 2);
					} else if (parameterName == "Textures") {
						// Three textures
						for (unsigned int textureNum = 0; textureNum < 3; textureNum++) {
							tableBuilder.addDescriptor(m_rtMeshHandles[blasIndex].textureHandles[textureNum].ptr, blasIndex * 2);
						}
					} else if (parameterName == "sys_brdfLUT") {
						tableBuilder.addDescriptor(m_rtBrdfLUTGPUHandle.ptr, blasIndex * 2);
					} else {
						Logger::Error("Unhandled root signature parameter! (" + parameterName + ")");
					}

					});
			}

			tableBuilder.addShader(L"NULL");
			blasIndex++;
		}

		m_hitGroupShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
	}
}

void DXRBase::createRaytracingPSO() {
	Memory::SafeRelease(m_rtPipelineState);

	DXRUtils::PSOBuilder psoBuilder;

	psoBuilder.addLibrary(ShaderPipeline::DEFAULT_SHADER_LOCATION + "dxr/" + m_shaderFilename + ".hlsl", { m_rayGenName, m_closestHitName, m_missName, m_closestProceduralPrimitive, m_intersectionProceduralPrimitive });
	psoBuilder.addHitGroup(m_hitGroupTriangleName, m_closestHitName);
	psoBuilder.addHitGroup(m_hitGroupMetaBallName, m_closestProceduralPrimitive, nullptr, m_intersectionProceduralPrimitive, D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE); //TODO: Add intesection Shader here!

	psoBuilder.addSignatureToShaders({ m_rayGenName }, m_localSignatureRayGen->get());
	psoBuilder.addSignatureToShaders({ m_hitGroupTriangleName }, m_localSignatureHitGroup_mesh->get());
	psoBuilder.addSignatureToShaders({ m_hitGroupMetaBallName }, m_localSignatureHitGroup_metaball->get());
	psoBuilder.addSignatureToShaders({ m_missName }, m_localSignatureMiss->get());

	psoBuilder.addLibrary(ShaderPipeline::DEFAULT_SHADER_LOCATION + "dxr/ShadowRay.hlsl", { m_shadowMissName });
	psoBuilder.addSignatureToShaders({ m_shadowMissName }, m_localSignatureEmpty->get());

	psoBuilder.setMaxPayloadSize(sizeof(RayPayload));
	psoBuilder.setMaxAttributeSize(sizeof(float) * 4);
	psoBuilder.setMaxRecursionDepth(MAX_RAY_RECURSION_DEPTH);
	psoBuilder.setGlobalSignature(m_dxrGlobalRootSignature->get());

	m_rtPipelineState = psoBuilder.build(m_context->getDevice());
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
	m_localSignatureRayGen->addDescriptorTable("gbufferInputTextures", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 0U, DX12GBufferRenderer::NUM_GBUFFERS + 1);
	m_localSignatureRayGen->addDescriptorTable("sys_brdfLUT", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5);
	m_localSignatureRayGen->addStaticSampler();

	m_localSignatureRayGen->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createHitGroupLocalRootSignature() {
	m_localSignatureHitGroup_mesh = std::make_unique<DX12Utils::RootSignature>("HitGroupLocal");
	m_localSignatureHitGroup_mesh->addDescriptorTable("sys_brdfLUT", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5);
	m_localSignatureHitGroup_mesh->addSRV("VertexBuffer", 1, 0);
	m_localSignatureHitGroup_mesh->addSRV("IndexBuffer", 1, 1);
	m_localSignatureHitGroup_mesh->addCBV("MeshCBuffer", 1, 0);
	m_localSignatureHitGroup_mesh->addDescriptorTable("Textures", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 3); // Textures (t0, t1, t2)
	m_localSignatureHitGroup_mesh->addStaticSampler();
	m_localSignatureHitGroup_mesh->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

	/*==========Metaballs=========*/
	m_localSignatureHitGroup_metaball = std::make_unique<DX12Utils::RootSignature>("HitGroupLocal2");
	m_localSignatureHitGroup_metaball->addDescriptorTable("sys_brdfLUT", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5);
	m_localSignatureHitGroup_metaball->addSRV("MetaballPositions", 1, 2);
	m_localSignatureHitGroup_metaball->addCBV("MeshCBuffer", 1, 0);
	m_localSignatureHitGroup_metaball->addStaticSampler();
	m_localSignatureHitGroup_metaball->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::createMissLocalRootSignature() {
	m_localSignatureMiss = std::make_unique<DX12Utils::RootSignature>("MissLocal");
	//m_localSignatureMiss->addDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3); // Skybox
	//m_localSignatureMiss->addStaticSampler();

	m_localSignatureMiss->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}

void DXRBase::initMetaballBuffers() {
	m_metaballPositions_srv.reserve(DX12API::NUM_SWAP_BUFFERS);
	for (size_t i = 0; i < DX12API::NUM_SWAP_BUFFERS; i++) {
		m_metaballPositions_srv.emplace_back(DX12Utils::CreateBuffer(m_context->getDevice(), MAX_NUM_METABALLS * sizeof(glm::vec3), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties));
	}
}

void DXRBase::createEmptyLocalRootSignature() {
	m_localSignatureEmpty = std::make_unique<DX12Utils::RootSignature>("EmptyLocal");
	m_localSignatureEmpty->build(m_context->getDevice(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
}
