#include "pch.h"
#include "DX12ConstantBuffer.h"

namespace ShaderComponent {

	ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot) {
		return new DX12ConstantBuffer(initData, size, bindShader, slot);
	}

	DX12ConstantBuffer::DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot) {

	}

	DX12ConstantBuffer::~DX12ConstantBuffer() {

	}

	void DX12ConstantBuffer::updateData(const void* newData, unsigned int bufferSize, unsigned int offset /*= 0U*/) {
		throw std::logic_error("The method or operation is not implemented.");
	}

	void DX12ConstantBuffer::bind() const {
		throw std::logic_error("The method or operation is not implemented.");
	}

}
