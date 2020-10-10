#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "../SVkAPI.h"

namespace ShaderComponent {

	class SVkConstantBuffer : public ConstantBuffer {
	public:
		SVkConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0, bool inComputeShader = false);
		~SVkConstantBuffer();
		
		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int offset = 0U) override;
		virtual void bind(void* cmdList) const override;

		const VkBuffer& getBuffer(unsigned int swapImageIndex) const;

	private:
		SVkAPI* m_context;

		std::vector<SVkAPI::BufferAllocation> m_uniformBuffers;
		std::vector<uint8_t*> m_mappedData;
		unsigned int m_bufferSize;
	};

}