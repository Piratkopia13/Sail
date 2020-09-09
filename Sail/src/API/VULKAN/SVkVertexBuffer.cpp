#include "pch.h"
#include "SVkVertexBuffer.h"
#include "Sail/Application.h"
#include "SVkUtils.h"
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

	SVkUtils::CreateBuffer(m_context->getDevice(), m_context->getPhysicalDevice(), getVertexBufferSize(), 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		m_vertexBuffer, m_vertexBufferMemory);

	// Copy vertex data from RAM into VRAM
	void* data;
	vkMapMemory(m_context->getDevice(), m_vertexBufferMemory, 0, getVertexBufferSize(), 0, &data);
	memcpy(data, vertices, (size_t)getVertexBufferSize());
	vkUnmapMemory(m_context->getDevice(), m_vertexBufferMemory);

}

SVkVertexBuffer::~SVkVertexBuffer() {
	vkDestroyBuffer(m_context->getDevice(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_context->getDevice(), m_vertexBufferMemory, nullptr);
}

void SVkVertexBuffer::bind(void* cmdList) {
	VkBuffer vertexBuffers[] = { m_vertexBuffer, m_vertexBuffer, m_vertexBuffer, m_vertexBuffer, m_vertexBuffer };
	VkDeviceSize offsets[5];
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