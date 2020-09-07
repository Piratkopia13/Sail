#include "pch.h"
#include "SVkVertexBuffer.h"
#include "Sail/Application.h"
//#include "VkUtils.h"

VertexBuffer* VertexBuffer::Create(const Mesh::Data& modelData, bool allowUpdates) {
	return SAIL_NEW SVkVertexBuffer(modelData, allowUpdates);
}

// When allowUpdates is enabled, there will exist one vertex buffer for each GPUBuffer,
// otherwise there will only be one buffer since it is not allowed to change between frames
SVkVertexBuffer::SVkVertexBuffer(const Mesh::Data& modelData, bool allowUpdates)
	: VertexBuffer(modelData)
{
	m_context = Application::getInstance()->getAPI<SVkAPI>();

	void* vertices = mallocVertexData(modelData);

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = static_cast<VkDeviceSize>(getVertexBufferSize());
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create a buffer object with no allocated memory
	if (vkCreateBuffer(m_context->getDevice(), &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS) {
		Logger::Error("Failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_context->getDevice(), m_vertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = m_context->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Allocate device memory
	if (vkAllocateMemory(m_context->getDevice(), &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS) {
		Logger::Error("Failed to allocate vertex buffer memory!");
	}

	// Bind the allocated memory to the buffer object
	vkBindBufferMemory(m_context->getDevice(), m_vertexBuffer, m_vertexBufferMemory, 0);

	// Copy vertex data from RAM into VRAM
	void* data;
	vkMapMemory(m_context->getDevice(), m_vertexBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices, (size_t)bufferInfo.size);
	vkUnmapMemory(m_context->getDevice(), m_vertexBufferMemory);

}

SVkVertexBuffer::~SVkVertexBuffer() {
	vkDestroyBuffer(m_context->getDevice(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_context->getDevice(), m_vertexBufferMemory, nullptr);
}

void SVkVertexBuffer::bind(void* cmdList) {
	VkBuffer vertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(*static_cast<VkCommandBuffer*>(cmdList), 0, 1, vertexBuffers, offsets);
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