#include "pch.h"
#include "SVkRTBase.h"
#include "../SVkVertexBuffer.h"
#include "../SVkIndexBuffer.h"

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

	//
	// Create BLAS
	//
	


	//
	// Create TLAS
	//

}

void SVkRTBase::createBLAS(const Mesh* mesh, VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd) {
	auto& allocator = m_context->getVmaAllocator();

	// Create blas construction info
	VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR };
	VkAccelerationStructureGeometryTrianglesDataKHR triangles = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
	VkAccelerationStructureGeometryKHR asGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	VkAccelerationStructureBuildOffsetInfoKHR offset = { };
	{
		geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometryInfo.maxPrimitiveCount = mesh->getNumIndices() / 3;
		geometryInfo.indexType = VK_INDEX_TYPE_UINT32;
		geometryInfo.maxVertexCount = mesh->getNumVertices();
		geometryInfo.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		geometryInfo.allowsTransforms = false;

		triangles.vertexFormat = geometryInfo.vertexFormat;
		triangles.vertexData.deviceAddress = static_cast<SVkVertexBuffer&>(mesh->getVertexBuffer()).getAddress();
		triangles.vertexStride = mesh->getVertexBuffer().getStride(); // DXRBase uses sizeof(vec3), but is that right for all meshes?
		triangles.indexType = geometryInfo.indexType;
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
		vmaCreateAccelerationStructure(allocator, &info, &vmaInfo, &m_blasAllocation.as, &m_blasAllocation.allocation, nullptr);

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
	SVkAPI::BufferAllocation scratchBufferAllocation;

	VkBufferCreateInfo scratchCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	scratchCreateInfo.size = maxScratch;
	scratchCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // TODO: this might not be right

	vmaCreateBuffer(allocator, &scratchCreateInfo, &allocInfo, &scratchBufferAllocation.buffer, &scratchBufferAllocation.allocation, nullptr);

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

	scratchBufferAllocation.destroy();
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
	vmaCreateAccelerationStructure(allocator, &asCreateInfo, &vmaInfo, &m_tlasAllocation.as, &m_tlasAllocation.allocation, nullptr);

	// Allocate a scratch buffer
	SVkAPI::BufferAllocation scratchBufferAllocation;

	VkBufferCreateInfo scratchCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	scratchCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // TODO: this might not be right

	vmaCreateAccelerationStructureScratchBuffer(allocator, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR, m_tlasAllocation.as, &scratchCreateInfo, &allocInfo, &scratchBufferAllocation.buffer, &scratchBufferAllocation.allocation, nullptr);

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
			instance.flags = 0;
			instance.accelerationStructureReference = blasAddress;
		}

		VkDeviceSize instanceDescsSize = 1 * sizeof(VkAccelerationStructureInstanceKHR); // TODO: change 1

		// Allocate the instance buffer
		VkBufferCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		instanceCreateInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		instanceCreateInfo.size = instanceDescsSize;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		// TODO: copy content to default buffer and delete staging buffer
		vmaCreateBuffer(allocator, &instanceCreateInfo, &allocInfo, &m_instancesAllocation.buffer, &m_instancesAllocation.allocation, nullptr);

		// Copy instance data to the buffer
		void* data;
		vmaMapMemory(allocator, m_instancesAllocation.allocation, &data);
		memcpy(data, &instance, instanceDescsSize);
		vmaUnmapMemory(allocator, m_instancesAllocation.allocation);

		// Get instance buffer device address
		bufferInfo.buffer = m_instancesAllocation.buffer;
		VkDeviceAddress instanceAddress = vkGetBufferDeviceAddress(m_context->getDevice(), &bufferInfo);

		// Make sure the copy of the instance buffer are copied before triggering the acceleration structure build
		VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

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

		scratchBufferAllocation.destroy();
	}
}
