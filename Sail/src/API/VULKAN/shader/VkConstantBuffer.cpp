#include "pch.h"
#include "VkConstantBuffer.h"
#include "Sail/Application.h"
//#include "../VkUtils.h"

namespace ShaderComponent {

	ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader) {
		return SAIL_NEW VkConstantBuffer(initData, size, bindShader, slot, inComputeShader);
	}

	VkConstantBuffer::VkConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader) {
		assert(false);
	}

	VkConstantBuffer::~VkConstantBuffer() {
		//delete[] m_needsUpdate;
	}

	void VkConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int meshIndex, unsigned int offset) {
		assert(false);
	}

	void VkConstantBuffer::bind(unsigned int meshIndex, void* cmdList) const {
		assert(false);
	}

	void VkConstantBuffer::reserve(unsigned int meshIndexMax) {
		assert(false);
	}

}
