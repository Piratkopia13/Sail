#pragma once

#include <vector>

class InputLayout {
public:
	// NOTE!
	// If InputType is changed then 
	// ResourceManager::getPSO, Mesh::getAttributeHash, ShaderParser::parse, InputLayout and PipelineStateObject::PipelineStateObject 
	// might have to be updated!
	enum InputType {
		POSITION = 1,
		TEXCOORD,
		NORMAL,
		TANGENT,
		BITANGENT
	};
	enum InputClassification {
		PER_VERTEX_DATA,
		PER_INSTANCE_DATA
	};
	
public:
	static InputLayout* InputLayout::Create();
	InputLayout();
	virtual ~InputLayout();

	// Set alignedByteOffset to -1 to align it automatically directly after the previous one
	virtual void pushFloat(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0);
	virtual void pushVec2(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0);
	virtual void pushVec3(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0);
	virtual void pushVec4(InputType inputType, const char* semanticName, unsigned int semanticIndex, unsigned int inputSlot, int alignedByteOffset = -1, InputClassification inputSlotClass = PER_VERTEX_DATA, unsigned int instanceDataStepRate = 0);
	
	virtual void create(void* vertexShaderBlob) = 0;
	virtual void bind() const = 0;

	const std::vector<InputType>& getOrderedInputs() const;
	unsigned int getVertexSize() const;
	unsigned int getInstanceSize() const;

protected:
	virtual int convertInputClassification(InputClassification inputSlotClass) = 0;

protected:
	std::vector<InputType> InputOrder;
	unsigned int VertexSize;
	unsigned int InstanceSize;

};