#pragma once

#include "Sail/graphics/shader/component/BindShader.h"

namespace ShaderComponent {

	class StructuredBuffer {
	public:
		static StructuredBuffer* StructuredBuffer::Create(void* initData, unsigned int size, unsigned int numElements, unsigned int stride, BIND_SHADER bindShader, unsigned int slot = 0);
		virtual ~StructuredBuffer() { }

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int numElements, unsigned int offset = 0U) = 0;

		virtual void bind(void* cmdList = nullptr) const = 0;
	};

}