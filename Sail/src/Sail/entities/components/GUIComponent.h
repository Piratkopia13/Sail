#pragma once
#include "Component.h"

class GUIComponent : public Component<GUIComponent> {
public:
	GUIComponent(Model* model) : m_model(model) { }
	~GUIComponent() { }

	Model* getModel() {
		return m_model;
	}

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
private:
	Model* m_model;
};
