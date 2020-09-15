#include "pch.h"
#include "SVkConstantBuffer.h"
#include "Sail/Application.h"
#include "../SVkUtils.h"

namespace ShaderComponent {

	ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader) {
		return SAIL_NEW SVkConstantBuffer(initData, size, bindShader, slot, inComputeShader);
	}

	SVkConstantBuffer::SVkConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader)
		: ConstantBuffer(slot)
	{
		m_context = Application::getInstance()->getAPI<SVkAPI>();
		auto numBuffers = m_context->getNumSwapChainImages();	// Num swap chain images could change after swap chain recreate / window resize
																// TODO: recreate buffers after swap chain recreation
		auto allocator = m_context->getVmaAllocator();

		VkDeviceSize bufferSize = size;

		m_uniformBuffers.resize(numBuffers);
		m_mappedData.resize(numBuffers);

		for (size_t i = 0; i < numBuffers; i++) {
			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			VmaAllocationCreateInfo allocInfo = {};
			// CPU only since we very often update the buffer data
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; // Map buffer as soon as it is created

			VmaAllocationInfo info{};
			vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_uniformBuffers[i].buffer, &m_uniformBuffers[i].allocation, &info);

			// Leave the buffer mapped for the lifetime of the instance
			m_mappedData[i] = info.pMappedData;

			// Place initData in the buffer
			memcpy(m_mappedData[i], initData, size);
		}
	}

	SVkConstantBuffer::~SVkConstantBuffer() { }

	void SVkConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int meshIndex, unsigned int offset) {
		auto currentImage = m_context->getSwapImageIndex();
		// TODO: only call this method when an update is required, and NOT every frame for every cbuffer in use
		memcpy((void*)((char*)m_mappedData[currentImage]+offset), newData, bufferSize);
	}

	void SVkConstantBuffer::bind(unsigned int meshIndex, void* cmdList) const {
		// Already bound through vkCmdBindDescriptorSets call done in SVkShader::bind()
	}

	const VkBuffer& SVkConstantBuffer::getBuffer(unsigned int swapImageIndex) const {
		return m_uniformBuffers[swapImageIndex].buffer;
	}

}
