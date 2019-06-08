#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "../DX12API.h"

namespace ShaderComponent {

	class DX12ConstantBuffer : public ConstantBuffer {
	public:
		DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0);
		~DX12ConstantBuffer();

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int offset = 0U) override;
		virtual void bind() const override;

	private:
		DX12API* m_context;

		void* m_newData;
		bool* m_needsUpdate;

		unsigned int m_location;
		wComPtr<ID3D12Resource1>* m_constantBufferUploadHeap;
		UINT8** m_cbGPUAddress;

	};

}