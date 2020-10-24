#include "pch.h"
#include "SVkRTBase.h"
#include "../SVkVertexBuffer.h"
#include "../SVkIndexBuffer.h"
#include "../shader/SVkPipelineStateObject.h"
#include "../shader/SVkShader.h"
#include "../renderer/SVkDeferredRenderer.h"

// Include defines shared with dxr shaders
#include "Sail/../../Demo/res/shaders/dxr/dxr.shared"
#include "Sail/graphics/light/LightSetup.h"
#include "../SVkUtils.h"

SVkRTBase::SVkRTBase() {
	m_context = Application::getInstance()->getAPI<SVkAPI>();

	auto numBuffers = m_context->getNumSwapBuffers();

	m_tlasAllocations.resize(numBuffers);
	m_instancesAllocations.resize(numBuffers);
	m_bottomInstances.resize(numBuffers);
	m_tlasScratchBufferAllocations.resize(numBuffers);
	m_blasScratchBufferAllocations.resize(numBuffers);

	// Get raytracing limit properties for this device
	if (m_context->supportsFeature(GraphicsAPI::RAYTRACING)) {
		VkPhysicalDeviceProperties2 props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		m_limitProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR };
		props.pNext = &m_limitProperties;
		vkGetPhysicalDeviceProperties2(m_context->getPhysicalDevice(), &props);
	}

	m_shader = static_cast<SVkShader*>(&Application::getInstance()->getResourceManager().getShaderSet(Shaders::RTShader));

	// Create Shader Binding Table
	// TODO: consider moving this to SVkShader
	{
		auto& allocator = m_context->getVmaAllocator();

		auto groupCount = 3; // 3 shaders: raygen, miss, chit. TODO: fetch this from the shader
		uint32_t groupHandleSize = m_limitProperties.shaderGroupHandleSize; // Size of a program identifier
		uint32_t baseAlignment = m_limitProperties.shaderGroupBaseAlignment; // Size of shader alignment

		uint32_t sbtSize = groupCount * baseAlignment;

		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		
		auto& pso = Application::getInstance()->getResourceManager().getPSO(m_shader);
		auto pipeline = static_cast<SVkPipelineStateObject&>(pso).getPipeline();

		vkGetRayTracingShaderGroupHandlesKHR(m_context->getDevice(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data());
		// Write the handles in the SBT
		
		VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCreateInfo.size = sbtSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VK_CHECK_RESULT(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &m_sbtAllocation.buffer, &m_sbtAllocation.allocation, nullptr));

		// Write the handles in the SBT
		void* mapped;
		vmaMapMemory(allocator, m_sbtAllocation.allocation, &mapped);
		auto* data = reinterpret_cast<uint8_t*>(mapped);
		for (uint32_t g = 0; g < groupCount; g++) {
			memcpy(data, shaderHandleStorage.data() + g * groupHandleSize, groupHandleSize); // raygen
			data += baseAlignment;
		}
		vmaUnmapMemory(allocator, m_sbtAllocation.allocation);
	}

}

SVkRTBase::~SVkRTBase() {
	vkDeviceWaitIdle(m_context->getDevice());
	m_sbtAllocation.destroy();
}

void SVkRTBase::update(const Renderer::RenderCommandList sceneGeometry, Camera* cam, LightSetup* lights, VkCommandBuffer cmd) {
	updateSceneCBuffer(cam, lights);

	auto swapIndex = m_context->getSwapIndex();
	uint32_t numInstances = 0;

	static auto flagFastTrace = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	static auto flagFastBuild = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
	static auto flagAllowUpdate = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;

	// Clear old instance lists
	// TODO: make this more efficient
	for (auto& it : m_bottomInstances[swapIndex]) {
		it.second.instanceTransforms.clear();
	}

	// Iterate render commands and build BLASes for meshes that are new since the last call to this method
	for (auto& renderCommand : sceneGeometry) {
		// Ignore command if no dxr flags are set
		if (!renderCommand.dxrFlags) continue;

		auto flags = flagFastTrace;
		if (renderCommand.dxrFlags & Renderer::DXRRenderFlag::MESH_DYNAMIC)
			flags = VkBuildAccelerationStructureFlagBitsKHR(flagFastBuild | flagAllowUpdate);

		auto& it = m_bottomInstances[swapIndex].find(renderCommand.mesh);
		bool hasMesh = (it != m_bottomInstances[swapIndex].end());
		if (!hasMesh) {
			auto& instanceList = addBLAS(renderCommand.mesh, flags);
			instanceList.instanceTransforms.emplace_back((glm::mat3x4)renderCommand.transform);
		} else {
			// TODO: make this a thing
			/*if (renderCommand.hasUpdatedSinceLastFrame) {
				rebuildBLAS()
			} else*/

			// Mesh already has a BLAS - add transform to instance list
			it->second.instanceTransforms.emplace_back((glm::mat3x4)renderCommand.transform);
		}
		numInstances++;
	}

	// Remove existing BLASes that are no longer in the scene
	// Currently O(n^2), TODO: make this more efficient
	for (auto it = std::begin(m_bottomInstances[swapIndex]); it != std::end(m_bottomInstances[swapIndex]);) {
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
			it = m_bottomInstances[swapIndex].erase(it);
		}
	}

	buildBLASes(cmd);
	buildTLAS(numInstances, flagFastTrace, cmd);

	/*iterate all meshes
		if !map[mesh]
			createBLAS(flags depending on static / dynamic)
		else
			if mesh vertices have updated since last frame
				rebuildBLAS(flags depending on static / dynamic)

		remove existing BLASes that are no longer in the scene
		createTLAS()
		update sbt if needed(? )*/

}

void SVkRTBase::dispatch(SVkRenderableTexture* outputTexture, Camera* cam, LightSetup* lights, VkCommandBuffer cmd) {

	auto swapIndex = m_context->getSwapIndex();
	auto& pso = static_cast<SVkPipelineStateObject&>(Application::getInstance()->getResourceManager().getPSO(m_shader));

	// Bind TLAS to shader descriptor set
	SVkShader::Descriptors descriptors;
	auto& as = descriptors.accelerationStructures.emplace_back();
	as.name = "rtScene";
	as.info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	as.info.accelerationStructureCount = 1;
	as.info.pAccelerationStructures = &m_tlasAllocations[swapIndex].as;

	auto& gbuffers = SVkDeferredRenderer::GetGBuffers();
	assert(gbuffers.positions && gbuffers.normals);

	// Bind GBuffer textures to set
	auto& gPositions = descriptors.images.emplace_back();
	gPositions.name = "RGGbuffer_positions";
	auto& gPositionsInfos = gPositions.infos.emplace_back();
	gPositionsInfos.imageView = gbuffers.positions->getView();
	gPositionsInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	auto& gNormals = descriptors.images.emplace_back();
	gNormals.name = "RGGbuffer_normals";
	auto& gNormalsInfos = gNormals.infos.emplace_back();
	gNormalsInfos.imageView = gbuffers.normals->getView();
	gNormalsInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Bind output texture to set
	auto& output = descriptors.images.emplace_back();
	output.name = "output";
	auto& outputInfo = output.infos.emplace_back();
	outputInfo.imageView = outputTexture->getView();
	outputInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	m_shader->updateDescriptors(descriptors, &pso);

	// Update cbuffer
	{
		DXRShaderCommon::SceneCBuffer newData = {};
		if (cam) {
			newData.cameraPosition = cam->getPosition();
			newData.projectionToWorld = glm::inverse(cam->getViewProjection());
			newData.viewToWorld = glm::inverse(cam->getViewMatrix());
		}

		if (lights) {
			newData.dirLightDirection = lights->getDirLight().direction;
		}
		m_shader->setCBuffer("RGSystemSceneBuffer", &newData, sizeof(newData), cmd);
	}

	pso.bind(cmd);

	// Set offsets into the SBT where the different shader groups can be found
	// TODO: make dynamic
	VkDeviceSize progSize = m_limitProperties.shaderGroupBaseAlignment; // Size of a program identifier
	VkDeviceSize rayGenOffset = 0u * progSize; // NOTE: hardcoded to first slot
	VkDeviceSize missOffset = 1u * progSize; // NOTE: hardcoded to second slot
	VkDeviceSize missStride = progSize;
	VkDeviceSize hitGroupOffset = 2u * progSize; // NOTE: hardcoded to third slot
	VkDeviceSize hitGroupStride = progSize;

	VkStridedBufferRegionKHR raygenSBT;
	raygenSBT.buffer = m_sbtAllocation.buffer;
	raygenSBT.offset = rayGenOffset;
	raygenSBT.stride = 0;
	raygenSBT.size = progSize;

	VkStridedBufferRegionKHR missSBT = raygenSBT;
	missSBT.offset = missOffset;
	missSBT.stride = missStride;

	VkStridedBufferRegionKHR hitSBT = raygenSBT;
	hitSBT.offset = hitGroupOffset;
	hitSBT.stride = hitGroupStride;

	VkStridedBufferRegionKHR callableSBT = {};

	auto* window = Application::getInstance()->getWindow();
	uint32_t width = window->getWindowWidth();
	uint32_t height = window->getWindowHeight();

	vkCmdTraceRaysKHR(cmd, &raygenSBT, &missSBT, &hitSBT, &callableSBT, width, height, 1);
}

SVkRTBase::InstanceList& SVkRTBase::addBLAS(const Mesh* mesh, VkBuildAccelerationStructureFlagBitsKHR flags) {
	auto swapIndex = m_context->getSwapIndex();

	// Create blas construction info
	auto& blasInfo = m_blasesToBuild.emplace_back();
	// We need to fill all variables in the BlasBuildInfo struct

	blasInfo.mesh = mesh;
	blasInfo.flags = flags;

	// Create the map entry
	auto& instanceList = m_bottomInstances[swapIndex].insert({ blasInfo.mesh, {} }).first->second;
	blasInfo.asAllocation = &instanceList.asAllocation;

	blasInfo.geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	blasInfo.geometryInfo.maxPrimitiveCount = (mesh->getNumIndices() > 0) ? mesh->getNumIndices() / 3 : mesh->getNumVertices() / 3;
	blasInfo.geometryInfo.indexType = (mesh->getNumIndices() > 0) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_NONE_KHR;
	blasInfo.geometryInfo.maxVertexCount = mesh->getNumVertices();
	blasInfo.geometryInfo.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	blasInfo.geometryInfo.allowsTransforms = false;

	auto& triangles = blasInfo.asGeometry.geometry.triangles;
	triangles.vertexFormat = blasInfo.geometryInfo.vertexFormat;
	triangles.vertexData.deviceAddress = static_cast<SVkVertexBuffer&>(mesh->getVertexBuffer()).getAddress();
	triangles.vertexStride = sizeof(glm::vec3); // Vertices are not interleaved, meaning all positions are located next to each other
	triangles.indexType = blasInfo.geometryInfo.indexType;
	if (mesh->getNumIndices() > 0)
		triangles.indexData.deviceAddress = static_cast<SVkIndexBuffer&>(mesh->getIndexBuffer()).getAddress();
	triangles.transformData = {}; // Optional

	blasInfo.asGeometry.geometryType = blasInfo.geometryInfo.geometryType;
	blasInfo.asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

	blasInfo.offset.primitiveCount = blasInfo.geometryInfo.maxPrimitiveCount;
	blasInfo.offset.primitiveOffset = 0;
	blasInfo.offset.firstVertex = 0;
	blasInfo.offset.transformOffset = 0;

	return instanceList;
}

void SVkRTBase::buildBLASes(VkCommandBuffer cmd) {
	// Create all blases contained in m_blasesToBuild

	if (m_blasesToBuild.empty()) return;

	auto swapIndex = m_context->getSwapIndex();
	auto& allocator = m_context->getVmaAllocator();

	//VkDeviceSize maxScratch = 0;
	std::vector<VkAccelerationStructureKHR*> blasRefs(m_blasesToBuild.size());
	std::vector<VkDeviceSize> scratchOffsets(m_blasesToBuild.size());
	VkDeviceSize scratchTotalSize = 0;
	uint32_t i = 0;
	for (auto& blasInfo : m_blasesToBuild) {
		//// Create the map entry
		//InstanceList& instance = m_bottomInstances[swapIndex].insert({ blasInfo.mesh, {} }).first->second;
		//instance.instanceTransforms.emplace_back(transformation);

		VkAccelerationStructureCreateInfoKHR info = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		info.flags = blasInfo.flags;
		info.maxGeometryCount = 1;
		info.pGeometryInfos = &blasInfo.geometryInfo;

		VmaAllocationCreateInfo vmaInfo = {};
		vmaInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		blasInfo.asAllocation->destroy(); // Destroy the old BLAS (if it exists)
		VK_CHECK_RESULT(vmaCreateAccelerationStructure(allocator, &info, &vmaInfo, &blasInfo.asAllocation->as, &blasInfo.asAllocation->allocation, nullptr));

		// Store a reference to the AS, this is used in the next loop where the actual build happens
		blasRefs[i] = &blasInfo.asAllocation->as;

		// Figure out how large the scratch buffer has to be for this guy
		{
			VkAccelerationStructureMemoryRequirementsInfoKHR memReqInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR };
			memReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
			memReqInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
			memReqInfo.accelerationStructure = blasInfo.asAllocation->as;

			VkMemoryRequirements2 memReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
			vkGetAccelerationStructureMemoryRequirementsKHR(m_context->getDevice(), &memReqInfo, &memReq);

			//maxScratch = std::max(maxScratch, memReq.memoryRequirements.size);

			// Align
			VkDeviceSize alignedSize = memReq.memoryRequirements.size;
			if (memReq.memoryRequirements.alignment != 0 && alignedSize % memReq.memoryRequirements.alignment != 0)
				alignedSize = alignedSize - (alignedSize % memReq.memoryRequirements.alignment) + memReq.memoryRequirements.alignment;

			assert(memReq.memoryRequirements.alignment == 0 || scratchTotalSize % memReq.memoryRequirements.alignment != 0 && "Memory alignment requirement differs between BLASes - this won't work with the current strategy.");

			scratchOffsets[i] = scratchTotalSize; // Requires all alignments to be the same
			scratchTotalSize += alignedSize;
		}

		i++;
	}

	// TODO: set a max buffer size, if the max is exceeded - run over multiple frames
	// NOTE: if the max size is set too low / too many blases are built every frame this could mean that some blases will never be built

	// NOTE: current implementation re-uses the largest scratch buffer
	//		 Consider testing if this is actually worth the vram-cost over the cost of creating a new buffer every time

	// Get the size of the current buffer
	VmaAllocationInfo allocInfo = {};
	if (m_blasScratchBufferAllocations[swapIndex].allocation)
		vmaGetAllocationInfo(allocator, m_blasScratchBufferAllocations[swapIndex].allocation, &allocInfo);

	if (!m_blasScratchBufferAllocations[swapIndex].allocation || scratchTotalSize > allocInfo.size) {
		// We need a larger scratch buffer than we currently have - destroy the current and allocate a larger one
		m_blasScratchBufferAllocations[swapIndex].destroy();

		VkBufferCreateInfo scratchCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		scratchCreateInfo.size = scratchTotalSize;
		scratchCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		Logger::Log("Allocating scratch buffer for BLASes with size: " + std::to_string(scratchTotalSize) + " Swap index: " + std::to_string(swapIndex));
		VK_CHECK_RESULT(vmaCreateBuffer(allocator, &scratchCreateInfo, &allocInfo, &m_blasScratchBufferAllocations[swapIndex].buffer, &m_blasScratchBufferAllocations[swapIndex].allocation, nullptr));
	}

	// Get the device address to the scratch buffer
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = m_blasScratchBufferAllocations[swapIndex].buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(m_context->getDevice(), &bufferInfo);

	// Build the BLASes simultaneously
	i = 0;
	for (auto& blasInfo : m_blasesToBuild) {
		auto* pGeometry = &blasInfo.asGeometry;
		VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.flags = blasInfo.flags;
		buildInfo.dstAccelerationStructure = *blasRefs[i];
		buildInfo.geometryArrayOfPointers = false;
		buildInfo.geometryCount = 1; // TODO: change?
		buildInfo.ppGeometries = &pGeometry;
		buildInfo.scratchData.deviceAddress = scratchAddress + scratchOffsets[i];

		auto* pOffset = &blasInfo.offset;
		vkCmdBuildAccelerationStructureKHR(cmd, 1, &buildInfo, &pOffset);

		i++;
	}
	// Barrier to ensure all builds are finished before using any of them
	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

	// TODO: perform compaction here

	// TODO: destroy scratch buffer when the BLAS has been created
	//scratchBufferAllocation.destroy();

	m_blasesToBuild.clear();
}

void SVkRTBase::buildTLAS(uint32_t numInstances, VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd) {
	auto swapIndex = m_context->getSwapIndex();
	auto& allocator = m_context->getVmaAllocator();

	auto& tlas = m_tlasAllocations[swapIndex];
	auto& instancesAlloc = m_instancesAllocations[swapIndex];

	VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR };
	geometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometryInfo.maxPrimitiveCount = numInstances; // Number of BLAS instances
	geometryInfo.allowsTransforms = true;

	VkAccelerationStructureCreateInfoKHR asCreateInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	asCreateInfo.flags = flags;
	asCreateInfo.maxGeometryCount = 1;
	asCreateInfo.pGeometryInfos = &geometryInfo;

	VmaAllocationCreateInfo vmaInfo = {};
	vmaInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	tlas.destroy(); // Destroy the old TLAS (if it exists)
	VK_CHECK_RESULT(vmaCreateAccelerationStructure(allocator, &asCreateInfo, &vmaInfo, &tlas.as, &tlas.allocation, nullptr));

	// Fetch scratch buffer size requirement and allocate a larger buffer if needed
	VkAccelerationStructureMemoryRequirementsInfoKHR memReqInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR };
	memReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
	memReqInfo.accelerationStructure = tlas.as;
	memReqInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;

	VkMemoryRequirements2 memReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };

	vkGetAccelerationStructureMemoryRequirementsKHR(m_context->getDevice(), &memReqInfo, &memReq);

	// Get the size of the current buffer
	VmaAllocationInfo allocInfo = {};
	if (m_tlasScratchBufferAllocations[swapIndex].allocation)
		vmaGetAllocationInfo(allocator, m_tlasScratchBufferAllocations[swapIndex].allocation, &allocInfo);

	if (!m_tlasScratchBufferAllocations[swapIndex].allocation || memReq.memoryRequirements.size > allocInfo.size) {
		// We need a larger scratch buffer than we currently have - destroy the current and allocate a larger one
		m_tlasScratchBufferAllocations[swapIndex].destroy();

		VkBufferCreateInfo scratchCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		scratchCreateInfo.size = memReq.memoryRequirements.size;
		scratchCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		Logger::Log("Allocating scratch buffer for TLAS with size: " + std::to_string(scratchCreateInfo.size) + " Swap index: " + std::to_string(swapIndex));
		VK_CHECK_RESULT(vmaCreateBuffer(allocator, &scratchCreateInfo, &allocInfo, &m_tlasScratchBufferAllocations[swapIndex].buffer, &m_tlasScratchBufferAllocations[swapIndex].allocation, nullptr));
	}

	// Get scratch buffer device address
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = m_tlasScratchBufferAllocations[swapIndex].buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(m_context->getDevice(), &bufferInfo);

	// Upload instance descriptions and build the tlas
	{
		std::vector<VkAccelerationStructureInstanceKHR> instances;
		instances.reserve(numInstances);
		// TODO: move this to a helper function(?)
		for (auto& it : m_bottomInstances[swapIndex]) {
			auto& instanceList = it.second;
			VkAccelerationStructureDeviceAddressInfoKHR addressInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
			addressInfo.accelerationStructure = instanceList.asAllocation.as;
			VkDeviceAddress blasAddress = vkGetAccelerationStructureDeviceAddressKHR(m_context->getDevice(), &addressInfo);
			
			// Iterate all instances using this BLAS
			for (auto& transform : instanceList.instanceTransforms) {
				auto& instance = instances.emplace_back();
				memcpy(&instance.transform, &transform, sizeof(instance.transform));
				instance.instanceCustomIndex = 0;
				instance.mask = 0xff;
				instance.instanceShaderBindingTableRecordOffset = 0;
				instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; // TODO: allow different flags
				instance.accelerationStructureReference = blasAddress;
			}
		}

		VkDeviceSize instanceDescsSize = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);

		//{
		//	// Allocate the instance buffer
		//	VkBufferCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		//	instanceCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		//	instanceCreateInfo.size = instanceDescsSize;

		//	VmaAllocationCreateInfo allocInfo = {};
		//	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		//	// TODO: copy content to default buffer and delete staging buffer
		//	// NOTE: when this is done, remember to change geometry.instances.data below
		//	m_instancesAllocation.destroy(); // Destroy the old buffer (if it exists)
		//	VK_CHECK_RESULT(vmaCreateBuffer(allocator, &instanceCreateInfo, &allocInfo, &m_instancesAllocation.buffer, &m_instancesAllocation.allocation, nullptr));

		//	SVkAPI::BufferAllocation stagingBuffer;
		//	m_context->scheduleOnGraphicsQueue([&](const VkCommandBuffer& cmd) {

		//		// Allocate a staging buffer
		//		VkBufferCreateInfo stagingCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		//		stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		//		stagingCreateInfo.size = instanceDescsSize;

		//		VmaAllocationCreateInfo allocInfo = {};
		//		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		//		VK_CHECK_RESULT(vmaCreateBuffer(allocator, &stagingCreateInfo, &allocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, nullptr));

		//		// Copy instance data to the buffer
		//		void* data;
		//		vmaMapMemory(allocator, stagingBuffer.allocation, &data);
		//		memcpy(data, &instance, instanceDescsSize);
		//		vmaUnmapMemory(allocator, stagingBuffer.allocation);

		//		// Copy from staging buffer to device local buffer
		//		VkBufferCopy copyRegion = {};
		//		copyRegion.size = instanceDescsSize;
		//		vkCmdCopyBuffer(cmd, stagingBuffer.buffer, m_instancesAllocation.buffer, 1, &copyRegion);

		//	});
		//	m_context->flushScheduledCommands();
		//	stagingBuffer.destroy();

		//	// Make sure the copy of the instance buffer are copied before triggering the acceleration structure build
		//	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		//	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		//	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		//}
		// cpu only memory
		// TODO: see if this is faster than uploading to GPU only memory through a staging buffer
		//		 Measure both tlas build times (should be slower) and dispatch times (should be faster)
		{
			// Allocate the instance buffer
			VkBufferCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			instanceCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			instanceCreateInfo.size = instanceDescsSize;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			// TODO: copy content to default buffer and delete staging buffer
			// NOTE: when this is done, remember to change geometry.instances.data below
			instancesAlloc.destroy(); // Destroy the old buffer (if it exists)
			VK_CHECK_RESULT(vmaCreateBuffer(allocator, &instanceCreateInfo, &allocInfo, &instancesAlloc.buffer, &instancesAlloc.allocation, nullptr));

			// Copy instance data to the buffer
			void* data;
			vmaMapMemory(allocator, instancesAlloc.allocation, &data);
			memcpy(data, instances.data(), instanceDescsSize);
			vmaUnmapMemory(allocator, instancesAlloc.allocation);

			// Make sure the copy of the instance buffer are copied before triggering the acceleration structure build
			VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		}

		// Get instance buffer device address
		bufferInfo.buffer = instancesAlloc.buffer;
		VkDeviceAddress instanceAddress = vkGetBufferDeviceAddress(m_context->getDevice(), &bufferInfo);
		
		// Build tlas
		VkAccelerationStructureGeometryDataKHR geometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
		geometry.instances.arrayOfPointers = false;
		geometry.instances.data.deviceAddress = instanceAddress;
		VkAccelerationStructureGeometryKHR tlasGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		tlasGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		tlasGeometry.geometry = geometry;

		auto* pGeometry = &tlasGeometry;
		VkAccelerationStructureBuildGeometryInfoKHR tlasInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		tlasInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		tlasInfo.flags = flags;
		tlasInfo.dstAccelerationStructure = tlas.as;
		tlasInfo.geometryArrayOfPointers = false;
		tlasInfo.geometryCount = 1;
		tlasInfo.ppGeometries = &pGeometry;
		tlasInfo.scratchData.deviceAddress = scratchAddress;

		VkAccelerationStructureBuildOffsetInfoKHR offset = {};
		offset.primitiveCount = numInstances;
		auto* pOffset = &offset;
		vkCmdBuildAccelerationStructureKHR(cmd, 1, &tlasInfo, &pOffset);
	}

	// Make sure TLAS build is done before it is used
	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void SVkRTBase::updateSceneCBuffer(Camera* cam, LightSetup* lights) {
	DXRShaderCommon::SceneCBuffer newData = {};
	if (cam) {
		newData.cameraPosition = cam->getPosition();
		newData.projectionToWorld = glm::inverse(cam->getViewProjection());
		newData.viewToWorld = glm::inverse(cam->getViewMatrix());
	}

	if (lights) {
		newData.dirLightDirection = lights->getDirLight().direction;
	}
	m_shader->setCBuffer("RGSystemSceneBuffer", &newData, sizeof(newData));
}
