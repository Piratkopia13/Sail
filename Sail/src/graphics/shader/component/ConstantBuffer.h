#pragma once

#include <Windows.h>
#include <d3d11.h>
#include "BindShader.h"

namespace ShaderComponent {

	class ConstantBuffer {

	public:
		ConstantBuffer(void* initData, UINT size, BIND_SHADER bindShader, UINT slot = 0);
		~ConstantBuffer();

		void updateData(const void* newData, UINT bufferSize, UINT offset = 0U);

		void bind();

		ID3D11Buffer* getBuffer() {
			return m_buffer;
		}

	private:
		ID3D11Buffer* m_buffer;
		BIND_SHADER m_bindShader;
		UINT m_slot;
		UINT m_bufferSize;
		void* m_data;

	};

}