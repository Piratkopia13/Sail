#pragma once

#include "Sail/graphics/shader/BindShader.h"

namespace ShaderComponent {

	class ConstantBuffer {
	public:
		static ConstantBuffer* ConstantBuffer::Create(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0);
		virtual ~ConstantBuffer() {}

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int offset = 0U) = 0;

		virtual void bind(void* cmdList = nullptr) const = 0;
	};

}