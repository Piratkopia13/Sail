#pragma once

#include "Sail/api/shader/Shader.h"
#include <atomic>
#include "Sail/events/Events.h"
class SVkAPI;

class SVkShader : public Shader, public IEventListener {
public:
	SVkShader(Shaders::ShaderSettings settings);
	~SVkShader();

	virtual void bind(void* cmdList) const override;

	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

	virtual bool setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) override;
	void setCBufferVar(const std::string& name, const void* data, unsigned int size) override;
	bool trySetCBufferVar(const std::string& name, const void* data, unsigned int size) override;

	bool onEvent(Event& event) override;

private:
	SVkAPI* m_context;
};