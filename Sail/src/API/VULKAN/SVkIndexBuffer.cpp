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
	auto allocator = m_context->getVmaAllocator();

	// Create cpu visible staging buffer
	{
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VK_CHECK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_stagingBuffer.buffer, &m_stagingBuffer.allocation, nullptr));
	}

	// Copy indices to the stating buffer
	void* data;
	vmaMapMemory(allocator, m_stagingBuffer.allocation, &data);
	memcpy(data, indices, (size_t)bufferSize);
	vmaUnmapMemory(allocator, m_stagingBuffer.allocation);

	// Create gpu local memory
	{
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT; // TODO: only do this if vulkan 1.2 / raytracing is used

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VK_CHECK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_indexBuffer.buffer, &m_indexBuffer.allocation, nullptr));
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
		vkCmdCopyBuffer(cmd, m_stagingBuffer.buffer, m_indexBuffer.buffer, 1, &copyRegion);
	}, uploadCompleteCallback);

	// Delete indices from cpu memory
	Memory::SafeDeleteArr(indices);

	// Store the device address (Used for ray tracing)
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = m_indexBuffer.buffer;
	m_address = vkGetBufferDeviceAddress(m_context->getDevice(), &bufferInfo);
}

SVkIndexBuffer::~SVkIndexBuffer() {
	vkDeviceWaitIdle(m_context->getDevice());
}

void SVkIndexBuffer::bind(void* cmdList) {
	vkCmdBindIndexBuffer(static_cast<VkCommandBuffer>(cmdList), m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

VkDeviceAddress SVkIndexBuffer::getAddress() {
	return m_address;
}
