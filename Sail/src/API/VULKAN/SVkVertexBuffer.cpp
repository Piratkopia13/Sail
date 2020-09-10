#include "pch.h"
#include "SVkVertexBuffer.h"
#include "Sail/Application.h"
#include "SVkUtils.h"
//#include "VkUtils.h"

VertexBuffer* VertexBuffer::Create(const Mesh::Data& modelData, bool allowUpdates) {
	return SAIL_NEW SVkVertexBuffer(modelData, allowUpdates);
}

// When allowUpdates is enabled, there will exist one vertex buffer for each GPUBuffer,
// otherwise there will only be one buffer since it is not allowed to change between frames.
// Using allowUpdates also creates the buffer on cpu shared (slower) memory.
SVkVertexBuffer::SVkVertexBuffer(const Mesh::Data& modelData, bool allowUpdates)
	: VertexBuffer(modelData)
	, m_allowUpdates(allowUpdates)
	, m_stagingBuffer(VK_NULL_HANDLE)
	, m_stagingBufferMemory(VK_NULL_HANDLE)
{
	m_context = Application::getInstance()->getAPI<SVkAPI>();
	auto numImages = m_context->getNumSwapChainImages();
	auto bufferSize = getVertexBufferSize();

	void* vertices = mallocVertexData(modelData);

	if (allowUpdates) {
		m_vertexBuffers.resize(numImages);
		m_vertexBufferMemories.resize(numImages);
		for (unsigned int i = 0; i < numImages; i++) {
			SVkUtils::CreateBuffer(m_context->getDevice(), m_context->getPhysicalDevice(), bufferSize, 
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				m_vertexBuffers[i], m_vertexBufferMemories[i]);
			
			// Copy vertex data from RAM into VRAM
			void* data;
			vkMapMemory(m_context->getDevice(), m_vertexBufferMemories[i], 0, bufferSize, 0, &data);
			memcpy(data, vertices, (size_t)bufferSize);
			vkUnmapMemory(m_context->getDevice(), m_vertexBufferMemories[i]);
		}
	} else {
		// allowUpdates == false

		// Create cpu visible staging buffer
		SVkUtils::CreateBuffer(m_context->getDevice(), m_context->getPhysicalDevice(), bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_stagingBuffer, m_stagingBufferMemory);

		// Copy vertices to the stating buffer
		void* data;
		vkMapMemory(m_context->getDevice(), m_stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices, (size_t)bufferSize);
		vkUnmapMemory(m_context->getDevice(), m_stagingBufferMemory);

		// Create gpu local memory
		m_vertexBuffers.resize(1);
		m_vertexBufferMemories.resize(1);
		SVkUtils::CreateBuffer(m_context->getDevice(), m_context->getPhysicalDevice(), bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffers[0], m_vertexBufferMemories[0]);

		auto uploadCompleteCallback = [&] {
			// Clean up staging buffer after copy is completed
			vkDestroyBuffer(m_context->getDevice(), m_stagingBuffer, nullptr);
			vkFreeMemory(m_context->getDevice(), m_stagingBufferMemory, nullptr);
		};

		// copy from staging to gpu local memory
		m_context->scheduleMemoryCopy([&, bufferSize](const VkCommandBuffer& cmd) {
			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = bufferSize;
			vkCmdCopyBuffer(cmd, m_stagingBuffer, m_vertexBuffers[0], 1, &copyRegion);
		}, uploadCompleteCallback);
	}

	// Delete vertices from cpu memory
	free(vertices);
}

SVkVertexBuffer::~SVkVertexBuffer() {
	vkDeviceWaitIdle(m_context->getDevice());
	for (auto& vb : m_vertexBuffers) {
		vkDestroyBuffer(m_context->getDevice(), vb, nullptr);
	}
	for (auto& vbm : m_vertexBufferMemories) {
		vkFreeMemory(m_context->getDevice(), vbm, nullptr);
	}
}

void SVkVertexBuffer::bind(void* cmdList) {
	// The first frame or so will bind the buffer before it has any valid data, this might still be fine(?)

	auto imageIndex = (m_allowUpdates) ? m_context->getSwapImageIndex() : 0;
	VkBuffer vertexBuffers[] = { m_vertexBuffers[imageIndex], m_vertexBuffers[imageIndex], m_vertexBuffers[imageIndex], m_vertexBuffers[imageIndex], m_vertexBuffers[imageIndex] };
	VkDeviceSize offsets[5];
	// Set offsets in the buffer
	// The order of the different vertex attributes is defined in VertexBuffer::mallocVertexData
	offsets[0] = 0;
	offsets[1] = getPositionsDataSize();
	offsets[2] = offsets[1] + getTexCoordsDataSize();
	offsets[3] = offsets[2] + getNormalsDataSize();
	offsets[4] = offsets[3] + getTangentsDataSize();

	vkCmdBindVertexBuffers(*static_cast<VkCommandBuffer*>(cmdList), 0, 5, vertexBuffers, offsets);
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