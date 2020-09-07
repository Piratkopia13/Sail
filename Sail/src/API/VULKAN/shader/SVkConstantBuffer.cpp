#include "pch.h"
#include "SVkConstantBuffer.h"
#include "Sail/Application.h"
//#include "../VkUtils.h"

namespace ShaderComponent {

	ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader) {
		return SAIL_NEW SVkConstantBuffer(initData, size, bindShader, slot, inComputeShader);
	}

	SVkConstantBuffer::SVkConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot, bool inComputeShader) {
		Logger::Warning("Tried to create a VK ConstantBuffer");
	}

	SVkConstantBuffer::~SVkConstantBuffer() {
		//delete[] m_needsUpdate;
	}

	void SVkConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int meshIndex, unsigned int offset) {
		assert(false);
	}

	void SVkConstantBuffer::bind(unsigned int meshIndex, void* cmdList) const {
		assert(false);
	}

	void SVkConstantBuffer::reserve(unsigned int meshIndexMax) {
		assert(false);
	}

}
