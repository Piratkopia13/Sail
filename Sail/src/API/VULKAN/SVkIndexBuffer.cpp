#include "pch.h"
#include "SVkIndexBuffer.h"
#include "Sail/Application.h"
#include "SVkUtils.h"

IndexBuffer* IndexBuffer::Create(Mesh::Data& modelData) {
	return SAIL_NEW SVkIndexBuffer(modelData);
}

SVkIndexBuffer::SVkIndexBuffer(Mesh::Data& modelData) 
	: IndexBuffer(modelData)
{
	m_context = Application::getInstance()->getAPI<SVkAPI>();
	unsigned long* indices = getIndexData(modelData);
	auto bufferSize = getIndexDataSize();

	// Create cpu visible staging buffer
	SVkUtils::CreateBuffer(m_context->getDevice(), m_context->getPhysicalDevice(), bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_stagingBuffer, m_stagingBufferMemory);

	// Copy vertices to the stating buffer
	void* data;
	vkMapMemory(m_context->getDevice(), m_stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices, (size_t)bufferSize);
	vkUnmapMemory(m_context->getDevice(), m_stagingBufferMemory);

	// Create gpu local memory
	SVkUtils::CreateBuffer(m_context->getDevice(), m_context->getPhysicalDevice(), bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

	auto uploadCompleteCallback = [&] {
		// Clean up staging buffer after copy is completed
		vkDestroyBuffer(m_context->getDevice(), m_stagingBuffer, nullptr);
		vkFreeMemory(m_context->getDevice(), m_stagingBufferMemory, nullptr);
	};

	// Copy from staging to gpu local memory
	m_context->scheduleMemoryCopy([&, bufferSize](const VkCommandBuffer& cmd) {
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(cmd, m_stagingBuffer, m_indexBuffer, 1, &copyRegion);
	}, uploadCompleteCallback);

	// Delete indices from cpu memory
	Memory::SafeDeleteArr(indices);
}

SVkIndexBuffer::~SVkIndexBuffer() {
	vkDeviceWaitIdle(m_context->getDevice());
	vkDestroyBuffer(m_context->getDevice(), m_indexBuffer, nullptr);
	vkFreeMemory(m_context->getDevice(), m_indexBufferMemory, nullptr);
}

void SVkIndexBuffer::bind(void* cmdList) {
	vkCmdBindIndexBuffer(*static_cast<VkCommandBuffer*>(cmdList), m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}