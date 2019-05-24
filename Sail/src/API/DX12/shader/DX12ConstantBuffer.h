#pragma once

#include "Sail/api/shader/ConstantBuffer.h"

namespace ShaderComponent {

	class DX12ConstantBuffer : public ConstantBuffer {
	public:
		DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0);
		~DX12ConstantBuffer();

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int offset = 0U) override;
		virtual void bind() const override;

	};

}