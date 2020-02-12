#pragma once

#include <d3d12.h>
#include "Sail/api/shader/InputLayout.h"

class DX12InputLayout : public InputLayout {
public:
	DX12InputLayout();
	~DX12InputLayout();

	virtual void pushFloat(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void pushVec2(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void pushVec3(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void pushVec4(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0) override;
	virtual void create(void* vertexShaderBlob) override;
	virtual void bind() const override;

	const D3D12_INPUT_LAYOUT_DESC& getDesc() const;

protected:
	virtual int convertInputClassification(InputClassification inputSlotClass) override;

private:
	void push(DXGI_FORMAT format, unsigned int typeSize, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset, InputClassification inputSlotClass, unsigned int instanceDataStepRate);

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs;
	D3D12_INPUT_LAYOUT_DESC m_inputLayoutDesc;

};