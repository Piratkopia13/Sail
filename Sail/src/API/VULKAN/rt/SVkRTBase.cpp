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

	// Get raytracing limit properties for this device
	if (m_context->supportsFeature(GraphicsAPI::RAYTRACING)) {
		VkPhysicalDeviceProperties2 props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		m_limitProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR };
		props.pNext = &m_limitProperties;
		vkGetPhysicalDeviceProperties2(m_context->getPhysicalDevice(), &props);
	}

	auto flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

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

void SVkRTBase::createBLAS(const Mesh* mesh, VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd) {
	auto& allocator = m_context->getVmaAllocator();

	// Create blas construction info
	VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR };
	VkAccelerationStructureGeometryKHR asGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	VkAccelerationStructureBuildOffsetInfoKHR offset = { };
	{
		geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometryInfo.maxPrimitiveCount = (mesh->getNumIndices() > 0) ? mesh->getNumIndices() / 3 : mesh->getNumVertices() / 3;
		geometryInfo.indexType = (mesh->getNumIndices() > 0) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_NONE_KHR;
		geometryInfo.maxVertexCount = mesh->getNumVertices();
		geometryInfo.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		geometryInfo.allowsTransforms = false;

		VkAccelerationStructureGeometryTrianglesDataKHR triangles = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
		triangles.vertexFormat = geometryInfo.vertexFormat;
		triangles.vertexData.deviceAddress = static_cast<SVkVertexBuffer&>(mesh->getVertexBuffer()).getAddress();
		triangles.vertexStride = sizeof(glm::vec3); // Vertices are not interleaved, meaning all positions are located next to each other
		triangles.indexType = geometryInfo.indexType;
		if (mesh->getNumIndices() > 0)
			triangles.indexData.deviceAddress = static_cast<SVkIndexBuffer&>(mesh->getIndexBuffer()).getAddress();
		triangles.transformData = {}; // Optional

		asGeometry.geometryType = geometryInfo.geometryType;
		asGeometry.geometry.triangles = triangles;
		asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

		offset.primitiveCount = geometryInfo.maxPrimitiveCount;
		offset.primitiveOffset = 0;
		offset.firstVertex = 0;
		offset.transformOffset = 0;
	}

	// Create blas
	// This should iterate all blas instances to be created
	VkDeviceSize maxScratch = 0;
	{
		VkAccelerationStructureCreateInfoKHR info = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		info.flags = flags;
		info.maxGeometryCount = 1;
		info.pGeometryInfos = &geometryInfo;

		VmaAllocationCreateInfo vmaInfo = {};
		vmaInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		m_blasAllocation.destroy(); // Destroy the old BLAS (if it exists)
		VK_CHECK_RESULT(vmaCreateAccelerationStructure(allocator, &info, &vmaInfo, &m_blasAllocation.as, &m_blasAllocation.allocation, nullptr));

		// Figure out how large the scratch buffer has to be for this guy
		{
			VkAccelerationStructureMemoryRequirementsInfoKHR memReqInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR };
			memReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
			memReqInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
			memReqInfo.accelerationStructure = m_blasAllocation.as;

			VkMemoryRequirements2 memReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
			vkGetAccelerationStructureMemoryRequirementsKHR(m_context->getDevice(), &memReqInfo, &memReq);

			maxScratch = std::max(maxScratch, memReq.memoryRequirements.size);
		}
	}

	// Allocate a scratch buffer using the maximum size for any single blas
	// This buffer will be used for all blas creations this frame
	static SVkAPI::BufferAllocation scratchBufferAllocation;

	VkBufferCreateInfo scratchCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	scratchCreateInfo.size = maxScratch;
	scratchCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // TODO: this might not be right

	VK_CHECK_RESULT(vmaCreateBuffer(allocator, &scratchCreateInfo, &allocInfo, &scratchBufferAllocation.buffer, &scratchBufferAllocation.allocation, nullptr));

	// Get the device address to the scratch buffer
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = scratchBufferAllocation.buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(m_context->getDevice(), &bufferInfo);

	// Builds the BLASes
	// This should iterate all blases
	{
		auto* pGeometry = &asGeometry;
		VkAccelerationStructureBuildGeometryInfoKHR blasInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		blasInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		blasInfo.flags = flags;
		blasInfo.dstAccelerationStructure = m_blasAllocation.as;
		blasInfo.geometryArrayOfPointers = false;
		blasInfo.geometryCount = 1; // TODO: change
		blasInfo.ppGeometries = &pGeometry;
		blasInfo.scratchData.deviceAddress = scratchAddress;

		auto* pOffset = &offset;
		vkCmdBuildAccelerationStructureKHR(cmd, 1, &blasInfo, &pOffset);
		// Since the scratch buffer is reused across builds, we need a barrier to ensure one build is finished before starting the next one
		VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
	}

	// TODO: perform compaction here

	// TODO: destroy scratch buffer when the BLAS has been created
	//scratchBufferAllocation.destroy();
}

void SVkRTBase::createTLAS(VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd) {
	auto& allocator = m_context->getVmaAllocator();

	VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR };
	geometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometryInfo.maxPrimitiveCount = 1; // TODO: change
	geometryInfo.allowsTransforms = true;

	VkAccelerationStructureCreateInfoKHR asCreateInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	asCreateInfo.flags = flags;
	asCreateInfo.maxGeometryCount = 1;
	asCreateInfo.pGeometryInfos = &geometryInfo;

	VmaAllocationCreateInfo vmaInfo = {};
	vmaInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	m_tlasAllocation.destroy(); // Destroy the old TLAS (if it exists)
	VK_CHECK_RESULT(vmaCreateAccelerationStructure(allocator, &asCreateInfo, &vmaInfo, &m_tlasAllocation.as, &m_tlasAllocation.allocation, nullptr));

	// Allocate a scratch buffer
	static SVkAPI::BufferAllocation scratchBufferAllocation;

	VkBufferCreateInfo scratchCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	scratchCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // TODO: this might not be right

	VK_CHECK_RESULT(vmaCreateAccelerationStructureScratchBuffer(allocator, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR, m_tlasAllocation.as, &scratchCreateInfo, &allocInfo, &scratchBufferAllocation.buffer, &scratchBufferAllocation.allocation, nullptr));

	// Get scratch buffer device address
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = scratchBufferAllocation.buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(m_context->getDevice(), &bufferInfo);

	// Upload instance descriptions and build the tlas
	{
		VkAccelerationStructureInstanceKHR instance = {};
		// TODO: move this to a helper function(?)
		{
			VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
			addressInfo.accelerationStructure = m_blasAllocation.as;
			VkDeviceAddress blasAddress = vkGetAccelerationStructureDeviceAddressKHR(m_context->getDevice(), &addressInfo);
			// The matrices for the instance transforms are row-major, instead of
			// column-major in the rest of the application
			glm::mat4 transp = glm::transpose(glm::mat4(1.0f)); // Id matrix only used for testing, replace with blas matrix
			// The instance transform value only contains 12 values, corresponding to a 4x3
			// matrix, hence saving the last row that is anyway always (0,0,0,1). Since
			// the matrix is row-major, we simply copy the first 12 values of the
			// original 4x4 matrix
			memcpy(&instance.transform, &transp, sizeof(instance.transform));
			instance.instanceCustomIndex = 0;
			instance.mask = 0xff;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = blasAddress;
		}

		VkDeviceSize instanceDescsSize = 1 * sizeof(VkAccelerationStructureInstanceKHR); // TODO: change 1

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
		{
			// Allocate the instance buffer
			VkBufferCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			instanceCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			instanceCreateInfo.size = instanceDescsSize;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			// TODO: copy content to default buffer and delete staging buffer
			// NOTE: when this is done, remember to change geometry.instances.data below
			m_instancesAllocation.destroy(); // Destroy the old buffer (if it exists)
			VK_CHECK_RESULT(vmaCreateBuffer(allocator, &instanceCreateInfo, &allocInfo, &m_instancesAllocation.buffer, &m_instancesAllocation.allocation, nullptr));

			// Copy instance data to the buffer
			void* data;
			vmaMapMemory(allocator, m_instancesAllocation.allocation, &data);
			memcpy(data, &instance, instanceDescsSize);
			vmaUnmapMemory(allocator, m_instancesAllocation.allocation);

			// Make sure the copy of the instance buffer are copied before triggering the acceleration structure build
			VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		}

		// Get instance buffer device address
		bufferInfo.buffer = m_instancesAllocation.buffer;
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
		tlasInfo.dstAccelerationStructure = m_tlasAllocation.as;
		tlasInfo.geometryArrayOfPointers = false;
		tlasInfo.geometryCount = 1;
		tlasInfo.ppGeometries = &pGeometry;
		tlasInfo.scratchData.deviceAddress = scratchAddress;

		VkAccelerationStructureBuildOffsetInfoKHR offset = {};
		offset.primitiveCount = 1;
		auto* pOffset = &offset;
		vkCmdBuildAccelerationStructureKHR(cmd, 1, &tlasInfo, &pOffset);

		// TODO: destroy scratch buffer when TLAS has been created
		//scratchBufferAllocation.destroy();
	}

	// Make sure TLAS build is done before it is used
	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void SVkRTBase::dispatch(SVkRenderableTexture* outputTexture, Camera* cam, LightSetup* lights, VkCommandBuffer cmd) {

	auto& pso = static_cast<SVkPipelineStateObject&>(Application::getInstance()->getResourceManager().getPSO(m_shader));

	// Bind TLAS to shader descriptor set
	SVkShader::Descriptors descriptors;
	auto& as = descriptors.accelerationStructures.emplace_back();
	as.name = "rtScene";
	as.info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	as.info.accelerationStructureCount = 1;
	as.info.pAccelerationStructures = &m_tlasAllocation.as;

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
