#pragma once

#include <d3d11.h>
#include <vector>
#include "Sail/api/shader/InputLayout.h"

class DX11InputLayout : public InputLayout {
public:
	DX11InputLayout();
	~DX11InputLayout();

	virtual void pushFloat(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass = PER_VERTEX_DATA, UINT instanceDataStepRate = 0) override;
	virtual void pushVec2(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass = PER_VERTEX_DATA, UINT instanceDataStepRate = 0) override;
	virtual void pushVec3(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass = PER_VERTEX_DATA, UINT instanceDataStepRate = 0) override;
	virtual void pushVec4(InputType inputType, LPCSTR semanticName, UINT semanticIndex, InputClassification inputSlotClass = PER_VERTEX_DATA, UINT instanceDataStepRate = 0) override;

	virtual void create(void* vertexShaderBlob) override;
	virtual void bind() const override;

protected:
	virtual int convertInputClassification(InputClassification inputSlotClass) override;

private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_ied;
	ID3D11InputLayout* m_inputLayout;
};
