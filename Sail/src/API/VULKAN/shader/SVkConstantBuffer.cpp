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

		VkDeviceSize bufferSize = size;

		m_uniformBuffers.resize(numBuffers);
		m_uniformBuffersMemory.resize(numBuffers);

		for (size_t i = 0; i < numBuffers; i++) {
			SVkUtils::CreateBuffer(m_context->getDevice(), m_context->getPhysicalDevice(), bufferSize, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				m_uniformBuffers[i], m_uniformBuffersMemory[i]);
		}

	}

	SVkConstantBuffer::~SVkConstantBuffer() {
		for (size_t i = 0; i < m_uniformBuffers.size(); i++) {
			vkDestroyBuffer(m_context->getDevice(), m_uniformBuffers[i], nullptr);
			vkFreeMemory(m_context->getDevice(), m_uniformBuffersMemory[i], nullptr);
		}
	}

	void SVkConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int meshIndex, unsigned int offset) {
		auto currentImage = m_context->getSwapImageIndex();

		// TODO: map on create and only memcpy here
		void* data;
		vkMapMemory(m_context->getDevice(), m_uniformBuffersMemory[currentImage], 0, bufferSize, 0, &data);
		memcpy(data, newData, bufferSize);
		vkUnmapMemory(m_context->getDevice(), m_uniformBuffersMemory[currentImage]);
	}

	void SVkConstantBuffer::bind(unsigned int meshIndex, void* cmdList) const {
		//assert(false);
	}

	const VkBuffer& SVkConstantBuffer::getBuffer(unsigned int swapImageIndex) const {
		return m_uniformBuffers[swapImageIndex];
	}

}
