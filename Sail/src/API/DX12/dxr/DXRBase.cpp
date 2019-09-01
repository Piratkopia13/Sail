#include "pch.h"
#include "DXRBase.h"
#include "Sail/Application.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "../DX12Utils.h"
#include "API/DX12/DX12VertexBuffer.h"
#include "API/DX12/DX12IndexBuffer.h"

const int DXRBase::MAX_RAY_RECURSION_DEPTH = 1;

DXRBase::DXRBase(const std::string& shaderFilename)
: m_shaderFilename(shaderFilename) 
{
	m_context = Application::getInstance()->getAPI<DX12API>();

	// Create frame resources (one per swap buffer)
	// Only one TLAS is used for the whole scene
	m_DXR_TopBuffer = m_context->createFrameResource<AccelerationStructureBuffers>();
	m_DXR_BottomBuffers = m_context->createFrameResource<std::vector<AccelerationStructureBuffers>>();
	m_rayGenShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();
	m_missShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();
	m_hitGroupShaderTable = m_context->createFrameResource<DXRUtils::ShaderTableData>();

	//createAccelerationStructures(cmdList); // TODO: make sure updateAS is called before the first dispatch (??)
	createDXRGlobalRootSignature();
	createRaytracingPSO();
	createShaderResources();
	createShaderTables();

}

DXRBase::~DXRBase() {

}

void DXRBase::updateAccelerationStructures(const std::vector<Renderer::RenderCommand>& sceneGeometry, ID3D12GraphicsCommandList4* cmdList) {

	// TODO: run this on an async compute queue
	createBLAS(sceneGeometry, cmdList);
	createTLAS(sceneGeometry, cmdList);

}

void DXRBase::updateCamera(Camera& cam) {
	CameraCBData newData = {};
	newData.cameraPosition = cam.getPosition();
	newData.projectionToWorld = glm::inverse(cam.getViewProjection());
	m_cameraCB->updateData(&newData, sizeof(newData));
}

ID3D12Resource* DXRBase::dispatch(ID3D12GraphicsCommandList4* cmdList) {
	
	unsigned int frameIndex = m_context->getFrameIndex();

	//// Update constant buffers
	//if (m_camera) {
	//	XMMATRIX jitterMat = XMMatrixIdentity();
	//	if (getRTFlags() & RT_ENABLE_JITTER_AA) {
	//		float jitterX = (float(m_dis(m_gen)) * 0.26f - 0.13f) / m_renderer->getWindow()->getWindowWidth();
	//		float jitterY = (float(m_dis(m_gen)) * 0.26f - 0.13f) / m_renderer->getWindow()->getWindowHeight();
	//		jitterMat = XMMatrixTranslation(jitterX, jitterY, 0.f);
	//	}
	//	m_sceneCBData->cameraPosition = m_camera->getPositionF3();
	//	m_sceneCBData->projectionToWorld = (m_camera->getInvProjMatrix() * jitterMat) * m_camera->getInvViewMatrix();
	//	m_sceneCB->setData(m_sceneCBData, 0);
	//}

	//m_rayGenCBData.frameCount++;
	//m_rayGenSettingsCB->setData(&m_rayGenCBData, 0);


	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_rtDescriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	// Let's raytrace
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_rtOutputUAV.resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

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
	cmdList->SetComputeRootSignature(m_dxrGlobalRootSignature.Get());

	// Set acceleration structure
	cmdList->SetComputeRootShaderResourceView(DXRGlobalRootParam::SRV_ACCELERATION_STRUCTURE, m_DXR_TopBuffer[m_context->getFrameIndex()].result->GetGPUVirtualAddress());
	// Set scene constant buffer
	cmdList->SetComputeRootConstantBufferView(DXRGlobalRootParam::CBV_SCENE_BUFFER, m_cameraCB->getBuffer()->GetGPUVirtualAddress());
	// Set ray gen settings constant buffer
	//cmdList->SetComputeRootConstantBufferView(DXRGlobalRootParam::CBV_SETTINGS, m_rayGenSettingsCB->getBuffer(m_context->getFrameIndex())->GetGPUVirtualAddress());

	// Dispatch
	cmdList->SetPipelineState1(m_rtPipelineState.Get());
	cmdList->DispatchRays(&raytraceDesc);

	return m_rtOutputUAV.resource.Get();

}

void DXRBase::createTLAS(const std::vector<Renderer::RenderCommand>& sceneGeometry, ID3D12GraphicsCommandList4* cmdList) {

	// Always rebuilds TLAS instead of updating it according to nvidia recommendations

	unsigned int numBLAS = sceneGeometry.size();
	unsigned int frameIndex = m_context->getFrameIndex();

	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = numBLAS;
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

	m_DXR_TopBuffer[frameIndex].instanceDesc = DX12Utils::CreateBuffer(m_context->getDevice(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * glm::max(numBLAS, 1U), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, DX12Utils::sUploadHeapProperties);
	m_DXR_TopBuffer[frameIndex].instanceDesc->SetName(L"TLAS_INSTANCE_DESC");

	D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc;
	m_DXR_TopBuffer[frameIndex].instanceDesc->Map(0, nullptr, (void**)& pInstanceDesc);

	for (UINT i = 0; i < numBLAS; i++) {

		pInstanceDesc->InstanceID = i;                            // exposed to the shader via InstanceID()
		pInstanceDesc->InstanceContributionToHitGroupIndex = i;   // offset inside the shader-table. we only have a single geometry, so the offset 0
		pInstanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		// apply transform from lambda function
		glm::mat3x4 m = sceneGeometry[i].transform;
		memcpy(pInstanceDesc->Transform, &m, sizeof(pInstanceDesc->Transform));

		pInstanceDesc->AccelerationStructure = m_DXR_BottomBuffers[frameIndex][i].result->GetGPUVirtualAddress();
		pInstanceDesc->InstanceMask = 0xFF;

		pInstanceDesc++;
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

void DXRBase::createBLAS(const std::vector<Renderer::RenderCommand>& sceneGeometry, ID3D12GraphicsCommandList4* cmdList) {

	unsigned int numBLAS = sceneGeometry.size();
	unsigned int frameIndex = m_context->getFrameIndex();
	bool onlyUpdate = false;

	if (!onlyUpdate) {
		//std::cout << "FULL BLAS BUILD" << std::endl;
		// Release old buffers if they exist
		for (auto& blas : m_DXR_BottomBuffers[frameIndex]) {
			blas.release();
		}
		// Resize all BLAS vectors
		m_DXR_BottomBuffers[frameIndex].resize(numBLAS);
	}

	for (unsigned int i = 0; i < numBLAS; i++) {
		auto& vb = static_cast<const DX12VertexBuffer&>(sceneGeometry[i].mesh->getVertexBuffer());
		auto& ib = static_cast<const DX12IndexBuffer&>(sceneGeometry[i].mesh->getIndexBuffer());

		D3D12_RAYTRACING_GEOMETRY_DESC geomDesc[1] = {};
		geomDesc[0] = {};
		geomDesc[0].Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geomDesc[0].Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geomDesc[0].Triangles.VertexBuffer.StartAddress = vb.getBuffer()->GetGPUVirtualAddress();
		geomDesc[0].Triangles.VertexBuffer.StrideInBytes = vb.getVertexDataStride();
		geomDesc[0].Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geomDesc[0].Triangles.VertexCount = sceneGeometry[i].mesh->getNumVertices();

		if (sceneGeometry[i].mesh->getNumIndices() > 0) {
			geomDesc[0].Triangles.IndexBuffer = ib.getBuffer()->GetGPUVirtualAddress();
			geomDesc[0].Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
			geomDesc[0].Triangles.IndexCount = UINT(sceneGeometry[i].mesh->getNumIndices());
		}

		// Get the size requirements for the scratch and AS buffers
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE; // Changing this flag depending on mesh can speed up performance significantly!
		/*if (onlyUpdate) {
			inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		}*/
		inputs.NumDescs = 1;
		inputs.pGeometryDescs = geomDesc;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
		m_context->getDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		// TODO: make sure buffer size is >= info.UpdateScratchDataSize in bytes
		if (!onlyUpdate) {
			// Create the buffers. They need to support UAV, and since we are going to immediately use them, we create them with an unordered-access state
			m_DXR_BottomBuffers[frameIndex][i].scratch = DX12Utils::CreateBuffer(m_context->getDevice(), info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::sDefaultHeapProps);
			m_DXR_BottomBuffers[frameIndex][i].scratch->SetName(L"BLAS_SCRATCH");
			m_DXR_BottomBuffers[frameIndex][i].result = DX12Utils::CreateBuffer(m_context->getDevice(), info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, DX12Utils::sDefaultHeapProps);
			m_DXR_BottomBuffers[frameIndex][i].result->SetName(L"BLAS_RESULT");
		}

		// Create the bottom-level AS
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
		asDesc.Inputs = inputs;
		asDesc.ScratchAccelerationStructureData = m_DXR_BottomBuffers[frameIndex][i].scratch->GetGPUVirtualAddress();
		asDesc.DestAccelerationStructureData = m_DXR_BottomBuffers[frameIndex][i].result->GetGPUVirtualAddress();
		if (inputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE)
			asDesc.SourceAccelerationStructureData = m_DXR_BottomBuffers[frameIndex][i].result->GetGPUVirtualAddress();

		cmdList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

		// We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
		D3D12_RESOURCE_BARRIER uavBarrier = {};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = m_DXR_BottomBuffers[frameIndex][i].result.Get();
		cmdList->ResourceBarrier(1, &uavBarrier);
	}

}

void DXRBase::createShaderResources() {

	// Create some resources only once on init
	if (!m_rtDescriptorHeap) {
		m_rtDescriptorHeap.Reset();
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 2000; // TODO: this does not throw error when full
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_context->getDevice()->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&m_rtDescriptorHeap));

		m_heapIncr = m_context->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Create the output resource. The dimensions and format should match the swap-chain
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.DepthOrArraySize = 1;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB formats can't be used with UAVs. We will convert to sRGB ourselves in the shader
		resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		resDesc.Width = Application::getInstance()->getWindow()->getWindowWidth();
		resDesc.Height = Application::getInstance()->getWindow()->getWindowHeight();
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.MipLevels = 1;
		resDesc.SampleDesc.Count = 1;
		m_context->getDevice()->CreateCommittedResource(&DX12Utils::sDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&m_rtOutputUAV.resource)); // Starting as copy-source to simplify onFrameRender()
		m_rtOutputUAV.resource->SetName(L"RTOutputUAV");

		// Create the UAV. Based on the root signature we created it should be the first entry
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_rtDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_rtDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		// Create a view for the output UAV
		m_context->getDevice()->CreateUnorderedAccessView(m_rtOutputUAV.resource.Get(), nullptr, &uavDesc, cpuHandle);
		m_rtOutputUAV.gpuHandle = gpuHandle;
		cpuHandle.ptr += m_heapIncr;
		gpuHandle.ptr += m_heapIncr;

		// Store heap start for views that might update in runtime
		m_rtHeapCPUHandle = cpuHandle;
		m_rtHeapGPUHandle = gpuHandle;

		//// Ray gen settings CB
		//m_rayGenCBData.flags = RT_ENABLE_TA | RT_ENABLE_JITTER_AA;
		//m_rayGenCBData.numAORays = 5;
		//m_rayGenCBData.AORadius = 0.9f;
		//m_rayGenCBData.frameCount = 0;
		//m_rayGenCBData.GISamples = 1;
		//m_rayGenCBData.GIBounces = 1;
		//m_rayGenSettingsCB = std::make_unique<DX12ConstantBuffer>("Ray Gen Settings CB", sizeof(RayGenSettings), m_renderer);
		//m_rayGenSettingsCB->setData(&m_rayGenCBData, 0);

		// Scene CB
		//m_sceneCBData = new SceneConstantBuffer();
		//m_sceneCB = std::make_unique<DX12ConstantBuffer>("Scene Constant Buffer", sizeof(SceneConstantBuffer), m_renderer);
		//m_sceneCB->setData(m_sceneCBData, 0/*Not used*/);

		unsigned int size = sizeof(CameraCBData);
		void* initData = malloc(size);
		memset(initData, 0, size);
		m_cameraCB = std::make_unique<ShaderComponent::DX12ConstantBuffer>(initData, size, ShaderComponent::BIND_SHADER::CS, 0);
		free(initData);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_rtHeapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_rtHeapGPUHandle;

	//// Create a view for each mesh textures
	//if (m_meshes) {
	//	m_rtMeshHandles.clear();
	//	for (auto& mesh : *m_meshes) {
	//		DX12Texture2DArray* texture = mesh->getTexture2DArray();
	//		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = texture->getSRVDesc();
	//		m_renderer->getDevice()->CreateShaderResourceView(texture->getResource(), &srvDesc, cpuHandle);

	//		MeshHandles handles;
	//		handles.vertexBufferHandle = static_cast<DX12VertexBuffer*>(mesh->geometryBuffer.vBuffer)->getBuffer()->GetGPUVirtualAddress();
	//		handles.indexBufferHandle = static_cast<DX12IndexBuffer*>(mesh->geometryBuffer.iBuffer)->getBuffer()->GetGPUVirtualAddress();
	//		handles.textureHandle = gpuHandle;

	//		handles.materialHandle = mesh->getMaterialCB()->getBuffer(0)->GetGPUVirtualAddress();

	//		m_rtMeshHandles.emplace_back(handles);

	//		cpuHandle.ptr += m_heapIncr;
	//		gpuHandle.ptr += m_heapIncr;
	//	}
	//}

}

void DXRBase::createShaderTables() {

	// 	 "Shader tables can be modified freely by the application (with appropriate state barriers)"

	//unsigned int frameIndex = m_context->getFrameIndex();
	// This might need to be called every AS update (without loop)
	for (unsigned int frameIndex = 0; frameIndex < m_context->getNumSwapBuffers(); frameIndex++) {
		// Ray gen
		{
			if (m_rayGenShaderTable[frameIndex].Resource) {
				m_rayGenShaderTable[frameIndex].Resource->Release();
				m_rayGenShaderTable[frameIndex].Resource.Reset();
			}
			DXRUtils::ShaderTableBuilder tableBuilder(m_rayGenName, m_rtPipelineState.Get());
			tableBuilder.addDescriptor(m_rtOutputUAV.gpuHandle.ptr);
			m_rayGenShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
		}

		// Miss
		{
			if (m_missShaderTable[frameIndex].Resource) {
				m_missShaderTable[frameIndex].Resource->Release();
				m_missShaderTable[frameIndex].Resource.Reset();
			}
			DXRUtils::ShaderTableBuilder tableBuilder(m_missName, m_rtPipelineState.Get());
			//tableBuilder.addDescriptor(m_skyboxGPUDescHandle.ptr);
			m_missShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
		}

		// Hit group
		{
			if (m_hitGroupShaderTable[frameIndex].Resource) {
				m_hitGroupShaderTable[frameIndex].Resource->Release();
				m_hitGroupShaderTable[frameIndex].Resource.Reset();
			}
			DXRUtils::ShaderTableBuilder tableBuilder(m_hitGroupName, m_rtPipelineState.Get());
			//for (unsigned int i = 0; i < m_meshes->size(); i++) {
			//	tableBuilder.addDescriptor(m_rtMeshHandles[i].vertexBufferHandle, i);
			//	tableBuilder.addDescriptor(m_rtMeshHandles[i].indexBufferHandle, i);
			//	tableBuilder.addDescriptor(m_rtMeshHandles[i].textureHandle.ptr, i); // only supports one texture/mesh atm // TODO FIX
			//	tableBuilder.addDescriptor(m_rtMeshHandles[i].materialHandle, i);
			//	//tableBuilder.addDescriptor(rayGenHandle, i);
			//}
			m_hitGroupShaderTable[frameIndex] = tableBuilder.build(m_context->getDevice());
		}
	}
}

void DXRBase::createRaytracingPSO() {
	m_localSignatureRayGen = createRayGenLocalRootSignature();
	m_localSignatureHitGroup = createHitGroupLocalRootSignature();
	m_localSignatureMiss = createMissLocalRootSignature();

	DXRUtils::PSOBuilder psoBuilder;
	psoBuilder.addLibrary(ShaderPipeline::DEFAULT_SHADER_LOCATION + "dxr/" + m_shaderFilename + ".hlsl", { m_rayGenName, m_closestHitName, m_missName });
	psoBuilder.addHitGroup(m_hitGroupName, m_closestHitName);
	psoBuilder.addSignatureToShaders({ m_rayGenName }, m_localSignatureRayGen.GetAddressOf());
	psoBuilder.addSignatureToShaders({ m_closestHitName }, m_localSignatureHitGroup.GetAddressOf());
	psoBuilder.addSignatureToShaders({ m_missName }, m_localSignatureMiss.GetAddressOf());
	psoBuilder.setMaxPayloadSize(sizeof(RayPayload));
	psoBuilder.setMaxRecursionDepth(MAX_RAY_RECURSION_DEPTH);
	psoBuilder.setGlobalSignature(m_dxrGlobalRootSignature.GetAddressOf());

	m_rtPipelineState = psoBuilder.build(m_context->getDevice());
}

void DXRBase::createDXRGlobalRootSignature() {
	D3D12_ROOT_PARAMETER rootParams[DXRGlobalRootParam::SIZE]{};

	//float RedChannel
	rootParams[DXRGlobalRootParam::FLOAT_RED_CHANNEL].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParams[DXRGlobalRootParam::FLOAT_RED_CHANNEL].Constants.RegisterSpace = 0;
	rootParams[DXRGlobalRootParam::FLOAT_RED_CHANNEL].Constants.ShaderRegister = 0;
	rootParams[DXRGlobalRootParam::FLOAT_RED_CHANNEL].Constants.Num32BitValues = 2; // ??

	// gRtScene
	rootParams[DXRGlobalRootParam::SRV_ACCELERATION_STRUCTURE].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParams[DXRGlobalRootParam::SRV_ACCELERATION_STRUCTURE].Descriptor.ShaderRegister = 0;
	rootParams[DXRGlobalRootParam::SRV_ACCELERATION_STRUCTURE].Descriptor.RegisterSpace = 0;

	// Scene CBV
	rootParams[DXRGlobalRootParam::CBV_SCENE_BUFFER].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[DXRGlobalRootParam::CBV_SCENE_BUFFER].Descriptor.ShaderRegister = 0;
	rootParams[DXRGlobalRootParam::CBV_SCENE_BUFFER].Descriptor.RegisterSpace = 2;

	// Ray Gen settings CBV
	rootParams[DXRGlobalRootParam::CBV_SETTINGS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[DXRGlobalRootParam::CBV_SETTINGS].Descriptor.ShaderRegister = 0;
	rootParams[DXRGlobalRootParam::CBV_SETTINGS].Descriptor.RegisterSpace = 1;
	rootParams[DXRGlobalRootParam::CBV_SETTINGS].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = _countof(rootParams);
	desc.pParameters = rootParams;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ID3DBlob* sigBlob;
	ID3DBlob* errorBlob;
	ThrowIfBlobError(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob), errorBlob);
	m_context->getDevice()->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_dxrGlobalRootSignature));
	m_dxrGlobalRootSignature->SetName(L"dxrGlobal");
}

ID3D12RootSignature* DXRBase::createRayGenLocalRootSignature() {
	D3D12_DESCRIPTOR_RANGE range[1]{};
	D3D12_ROOT_PARAMETER rootParams[DXRRayGenRootParam::SIZE]{};

	// lOutput
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;
	range[0].RegisterSpace = 0;
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParams[DXRRayGenRootParam::DT_UAV_OUTPUT].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[DXRRayGenRootParam::DT_UAV_OUTPUT].DescriptorTable.NumDescriptorRanges = _countof(range);
	rootParams[DXRRayGenRootParam::DT_UAV_OUTPUT].DescriptorTable.pDescriptorRanges = range;

	// Create the desc
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = _countof(rootParams);
	desc.pParameters = rootParams;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;


	ID3DBlob* sigBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	ThrowIfBlobError(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob), errorBlob);
	ID3D12RootSignature* pRootSig;
	ThrowIfFailed(m_context->getDevice()->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig)));
	pRootSig->SetName(L"RayGenLocal");

	return pRootSig;
}

ID3D12RootSignature* DXRBase::createHitGroupLocalRootSignature() {
	D3D12_ROOT_PARAMETER rootParams[DXRHitGroupRootParam::SIZE]{};

	rootParams[DXRHitGroupRootParam::SRV_VERTEX_BUFFER].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParams[DXRHitGroupRootParam::SRV_VERTEX_BUFFER].Descriptor.ShaderRegister = 1;
	rootParams[DXRHitGroupRootParam::SRV_VERTEX_BUFFER].Descriptor.RegisterSpace = 0;

	// diffuseTexture
	D3D12_DESCRIPTOR_RANGE range[1]{};
	range[0].BaseShaderRegister = 2;
	range[0].NumDescriptors = 1;
	range[0].RegisterSpace = 0;
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParams[DXRHitGroupRootParam::SRV_VERTEX_BUFFER].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParams[DXRHitGroupRootParam::SRV_VERTEX_BUFFER].Descriptor.ShaderRegister = 1;
	rootParams[DXRHitGroupRootParam::SRV_VERTEX_BUFFER].Descriptor.RegisterSpace = 0;

	rootParams[DXRHitGroupRootParam::SRV_INDEX_BUFFER].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParams[DXRHitGroupRootParam::SRV_INDEX_BUFFER].Descriptor.ShaderRegister = 1;
	rootParams[DXRHitGroupRootParam::SRV_INDEX_BUFFER].Descriptor.RegisterSpace = 1;

	rootParams[DXRHitGroupRootParam::DT_TEXTURES].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[DXRHitGroupRootParam::DT_TEXTURES].DescriptorTable.NumDescriptorRanges = _countof(range);
	rootParams[DXRHitGroupRootParam::DT_TEXTURES].DescriptorTable.pDescriptorRanges = range;

	// Material properties CBV
	rootParams[DXRHitGroupRootParam::CBV_MATERIAL].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[DXRHitGroupRootParam::CBV_MATERIAL].Descriptor.ShaderRegister = 1;
	rootParams[DXRHitGroupRootParam::CBV_MATERIAL].Descriptor.RegisterSpace = 0;
	rootParams[DXRHitGroupRootParam::CBV_MATERIAL].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC staticSamplerDesc = {};
	staticSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.MipLODBias = 0.f;
	staticSamplerDesc.MaxAnisotropy = 1;
	staticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSamplerDesc.MinLOD = 0.f;
	staticSamplerDesc.MaxLOD = FLT_MAX;
	staticSamplerDesc.RegisterSpace = 0;
	staticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = _countof(rootParams);
	desc.pParameters = rootParams;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	desc.NumStaticSamplers = 1;
	desc.pStaticSamplers = &staticSamplerDesc;


	ID3DBlob* sigBlob;
	ID3DBlob* errorBlob;
	ThrowIfBlobError(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob), errorBlob);
	ID3D12RootSignature* rootSig;
	m_context->getDevice()->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSig));
	rootSig->SetName(L"HitGroupLocal");

	return rootSig;
}

ID3D12RootSignature* DXRBase::createMissLocalRootSignature() {
	D3D12_ROOT_PARAMETER rootParams[DXRMissRootParam::SIZE]{};

	D3D12_DESCRIPTOR_RANGE range[1]{};
	range[0].BaseShaderRegister = 3;
	range[0].NumDescriptors = 1;
	range[0].RegisterSpace = 0;
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParams[DXRMissRootParam::SRV_SKYBOX].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[DXRMissRootParam::SRV_SKYBOX].DescriptorTable.NumDescriptorRanges = _countof(range);
	rootParams[DXRMissRootParam::SRV_SKYBOX].DescriptorTable.pDescriptorRanges = range;

	D3D12_STATIC_SAMPLER_DESC staticSamplerDesc = {};
	staticSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.MipLODBias = 0.f;
	staticSamplerDesc.MaxAnisotropy = 1;
	staticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSamplerDesc.MinLOD = 0.f;
	staticSamplerDesc.MaxLOD = FLT_MAX;
	staticSamplerDesc.RegisterSpace = 0;
	staticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = _countof(rootParams);
	desc.pParameters = rootParams;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	desc.NumStaticSamplers = 1;
	desc.pStaticSamplers = &staticSamplerDesc;

	ID3DBlob* sigBlob;
	ID3DBlob* errorBlob;
	ThrowIfBlobError(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob), errorBlob);
	ID3D12RootSignature* pRootSig;
	m_context->getDevice()->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig));
	pRootSig->SetName(L"MissLocal");

	return pRootSig;
}
