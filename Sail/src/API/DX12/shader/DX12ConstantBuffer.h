#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "../DX12API.h"

namespace ShaderComponent {

	// TODO: think about renaming this class to DX12ResourceHeap as it a cbuffer for each mesh in the scene
	class DX12ConstantBuffer : public ConstantBuffer {
	public:
		DX12ConstantBuffer(void* initData, unsigned int size, BIND_SHADER bindShader, unsigned int slot = 0);
		~DX12ConstantBuffer();

		virtual void updateData(const void* newData, unsigned int bufferSize, unsigned int offset = 0U) override;
		virtual void bind(void* cmdList) const override;

		void setResourceHeapMeshIndex(unsigned int index);

	private:
		void createBuffers();

	private:
		DX12API* m_context;

		void* m_newData;
		bool* m_needsUpdate;

		// The index specifies which offset it should write/read from in the resource heap
		// This is used since multiple meshes store their cbuffers in a single resource heap
		unsigned int m_resourceHeapMeshIndex;
		unsigned int m_byteAlignedSize;
		unsigned int m_resourceHeapSize;

		unsigned int m_register;
		wComPtr<ID3D12Resource1>* m_constantBufferUploadHeap;
		UINT8** m_cbGPUAddress;

	};

}