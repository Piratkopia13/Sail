#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "../VkAPI.h"

namespace ShaderComponent {

	class VkConstantBuffer : public ConstantBuffer {
	public:
		VkConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0, bool inComputeShader = false);
		~VkConstantBuffer();

		// meshIndex specifies which offset to write/read from in the resource heap
		// This is used since all meshes rendered this frame store data the same cbuffer
		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int meshIndex, unsigned int offset = 0U) override;
		virtual void bind(unsigned int meshIndex, void* cmdList) const override;

		// Increases the size of the current frame's buffer (if needed) to 
		// allow for "meshIndexMax"-number of meshes to be rendered this frame
		// Note: Buffers can not decrease in size
		void reserve(unsigned int meshIndexMax);

	private:
		VkAPI* m_context;
	};

}