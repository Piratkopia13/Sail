#pragma once

#include "Sail/graphics/shader/BindShader.h"

namespace ShaderComponent {

	class ConstantBuffer {
	public:
		static ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0, bool inComputeShader = false);
		ConstantBuffer(unsigned int slot) { this->slot = slot; }
		virtual ~ConstantBuffer() {}

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int meshIndex = 0U, unsigned int offset = 0U) = 0;

		virtual void bind(unsigned int meshIndex = 0U, void* cmdList = nullptr) const = 0;

		unsigned int getSlot() const { return this->slot; }

	protected:
		unsigned int slot;
	};

}