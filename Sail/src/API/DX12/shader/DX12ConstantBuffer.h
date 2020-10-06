#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "../DX12API.h"

namespace ShaderComponent {

	class DX12ConstantBuffer : public ConstantBuffer {
	public:
		DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0, bool inComputeShader = false);
		~DX12ConstantBuffer();

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int offset = 0U) override;
		virtual void bind(void* cmdList) const override;

		ID3D12Resource* getBuffer() const;

	private:
		void createBuffer(unsigned int swapIndex);

	private:
		DX12API* m_context;

		unsigned int m_byteAlignedSize;
		std::vector<unsigned int> m_resourceHeapSizes;

		bool m_inComputeShader;
		std::vector<wComPtr<ID3D12Resource>> m_constantBufferUploadHeap;
		std::vector<UINT8*> m_cbGPUAddress;
	};

}