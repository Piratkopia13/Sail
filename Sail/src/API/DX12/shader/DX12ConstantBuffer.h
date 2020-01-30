#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "../DX12API.h"

namespace ShaderComponent {

	// TODO: think about renaming this class to DX12ResourceHeap as it is a cbuffer for each mesh in the scene
	class DX12ConstantBuffer : public ConstantBuffer {
	public:
		DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0);
		~DX12ConstantBuffer();

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int offset = 0U) override;
		virtual void bind(void* cmdList) const override;

		// Increases the size of the current frame's buffer (if needed) to 
		// allow for "meshIndexMax"-number of meshes to be rendered this frame
		// Note: Buffers can not decrease in size
		void reserve(unsigned int meshIndexMax);

		// Sets the mesh index to be used by all following cbuffer method calls
		// Note: This is not at all thread safe!!
		void setResourceHeapMeshIndex(unsigned int index);

		ID3D12Resource* getBuffer() const;

	private:
		void createBuffer(unsigned int swapIndex);

	private:
		DX12API* m_context;

		// The index specifies which offset it should write/read from in the resource heap
		// This is used since multiple meshes store their cbuffers in a single resource heap
		unsigned int m_resourceHeapMeshIndex;
		unsigned int m_byteAlignedSize;
		std::vector<unsigned int> m_resourceHeapSizes;

		unsigned int m_register;
		std::vector<wComPtr<ID3D12Resource>> m_constantBufferUploadHeap;
		std::vector<UINT8*> m_cbGPUAddress;

	};

}