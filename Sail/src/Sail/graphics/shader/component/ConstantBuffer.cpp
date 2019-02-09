#include "pch.h"
#include "ConstantBuffer.h"
#include "Sail/Application.h"

namespace ShaderComponent {

	ConstantBuffer::ConstantBuffer(void* initData, UINT size, BIND_SHADER bindShader, UINT slot)
		: m_bindShader(bindShader)
		, m_slot(slot)
		, m_bufferSize(size)
	{

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.ByteWidth = size;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = initData;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateBuffer(&desc, &data, &m_buffer));

		// Keep a copy of the data on the CPU
		// Reason being GPU buffer is write only, and updateData can write parts of the buffer at a time
		m_data = malloc(size);
		memcpy(m_data, initData, size);

	}
	ConstantBuffer::~ConstantBuffer() {
		Memory::safeRelease(m_buffer);
		free(m_data);
	}

	void ConstantBuffer::updateData(const void* newData, UINT bufferSize, UINT offset) {

		// Overwrite part of the locally stored data
		memcpy((char*)m_data + offset, newData, bufferSize);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Application::getInstance()->getAPI()->getDeviceContext()->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		std::memcpy(mappedResource.pData, m_data, m_bufferSize);
		Application::getInstance()->getAPI()->getDeviceContext()->Unmap(m_buffer, 0);

	}

	void ConstantBuffer::bind() {
		if (m_bindShader & ShaderComponent::VS)
			Application::getInstance()->getAPI()->getDeviceContext()->VSSetConstantBuffers(m_slot, 1, &m_buffer);
		if (m_bindShader & ShaderComponent::HS)
			Application::getInstance()->getAPI()->getDeviceContext()->HSSetConstantBuffers(m_slot, 1, &m_buffer);
		if (m_bindShader & ShaderComponent::DS)
			Application::getInstance()->getAPI()->getDeviceContext()->DSSetConstantBuffers(m_slot, 1, &m_buffer);
		if (m_bindShader & ShaderComponent::PS)
			Application::getInstance()->getAPI()->getDeviceContext()->PSSetConstantBuffers(m_slot, 1, &m_buffer);
		if (m_bindShader & ShaderComponent::GS)
			Application::getInstance()->getAPI()->getDeviceContext()->GSSetConstantBuffers(m_slot, 1, &m_buffer);
		if (m_bindShader & ShaderComponent::CS)
			Application::getInstance()->getAPI()->getDeviceContext()->CSSetConstantBuffers(m_slot, 1, &m_buffer);
	}

}