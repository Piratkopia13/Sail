#include "pch.h"
#include "SVkVertexBuffer.h"
#include "Sail/Application.h"
#include "SVkUtils.h"

VertexBuffer* VertexBuffer::Create(const Mesh::Data& modelData, bool allowUpdates) {
	return SAIL_NEW SVkVertexBuffer(modelData, allowUpdates);
}

// When allowUpdates is enabled, there will exist one vertex buffer for each GPUBuffer,
// otherwise there will only be one buffer since it is not allowed to change between frames.
// Using allowUpdates also creates the buffer on cpu shared (slower) memory.
SVkVertexBuffer::SVkVertexBuffer(const Mesh::Data& modelData, bool allowUpdates)
	: VertexBuffer(modelData)
	, m_allowUpdates(allowUpdates)
{
	m_context = Application::getInstance()->getAPI<SVkAPI>();
	auto numImages = m_context->getNumSwapchainImages();
	auto bufferSize = getVertexBufferSize();
	auto allocator = m_context->getVmaAllocator();

	void* vertices = mallocVertexData(modelData);

	if (allowUpdates) {
		m_vertexBuffers.resize(numImages);
		for (unsigned int i = 0; i < numImages; i++) {
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

			VK_CHECK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_vertexBuffers[i].buffer, &m_vertexBuffers[i].allocation, nullptr));
			
			// Copy vertex data from RAM into VRAM
			void* data;
			vmaMapMemory(allocator, m_vertexBuffers[i].allocation, &data);
			memcpy(data, vertices, (size_t)bufferSize);
			vmaUnmapMemory(allocator, m_vertexBuffers[i].allocation);
		}
	} else {
		// allowUpdates == false

		// Create cpu visible staging buffer
		{
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

			VK_CHECK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_stagingBuffer.buffer, &m_stagingBuffer.allocation, nullptr));
		}

		// Copy vertices to the staging buffer
		void* data;
		vmaMapMemory(allocator, m_stagingBuffer.allocation, &data);
		memcpy(data, vertices, (size_t)bufferSize);
		vmaUnmapMemory(allocator, m_stagingBuffer.allocation);

		// Create gpu local memory
		m_vertexBuffers.resize(1);
		{
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			VK_CHECK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_vertexBuffers[0].buffer, &m_vertexBuffers[0].allocation, nullptr));
		}

		auto uploadCompleteCallback = [&] {
			// Clean up staging buffer after copy is completed
			m_stagingBuffer.destroy();
		};

		// Copy from staging to gpu local memory
		m_context->scheduleOnCopyQueue([&, bufferSize](const VkCommandBuffer& cmd) {
			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = bufferSize;
			vkCmdCopyBuffer(cmd, m_stagingBuffer.buffer, m_vertexBuffers[0].buffer, 1, &copyRegion);
		}, uploadCompleteCallback);
	}

	// Delete vertices from cpu memory
	free(vertices);
}

SVkVertexBuffer::~SVkVertexBuffer() {
	vkDeviceWaitIdle(m_context->getDevice());
}

void SVkVertexBuffer::bind(void* cmdList) {
	// The first frame or so will bind the buffer before it has any valid data, this might still be fine(?)

	auto imageIndex = (m_allowUpdates) ? m_context->getSwapImageIndex() : 0;
	VkBuffer vertexBuffers[] = { m_vertexBuffers[imageIndex].buffer, m_vertexBuffers[imageIndex].buffer, m_vertexBuffers[imageIndex].buffer, m_vertexBuffers[imageIndex].buffer, m_vertexBuffers[imageIndex].buffer };
	VkDeviceSize offsets[5];
	// Set offsets in the buffer
	// The order of the different vertex attributes is defined in VertexBuffer::mallocVertexData
	offsets[0] = 0;
	offsets[1] = getPositionsDataSize();
	offsets[2] = offsets[1] + getTexCoordsDataSize();
	offsets[3] = offsets[2] + getNormalsDataSize();
	offsets[4] = offsets[3] + getTangentsDataSize();

	vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(cmdList), 0, 5, vertexBuffers, offsets);
}

void SVkVertexBuffer::update(Mesh::Data& data) {
	assert(false);
}

void SVkVertexBuffer::setAsUpdated() {
	assert(false);
}

bool SVkVertexBuffer::hasBeenUpdated() const {
	assert(false);
	return false;
}

void SVkVertexBuffer::resetHasBeenUpdated() { 
	assert(false);
}