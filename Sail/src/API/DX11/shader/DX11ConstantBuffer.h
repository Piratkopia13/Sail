#pragma once

#include <d3d11.h>
#include "Sail/api/shader/ConstantBuffer.h"

namespace ShaderComponent {

	class DX11ConstantBuffer : public ConstantBuffer {
	public:
		DX11ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0);
		virtual ~DX11ConstantBuffer();

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int meshIndex = 0U, unsigned int offset = 0U) override;

		virtual void bind(unsigned int meshIndex = 0U, void* cmdList = nullptr) const override;

	private:
		ID3D11Buffer* m_buffer;
		BIND_SHADER m_bindShader;
		unsigned int m_slot;
		unsigned int m_bufferSize;
		void* m_data;
	};

}