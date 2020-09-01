#pragma once

#include "Sail/api/shader/Shader.h"
#include <atomic>
#include "Sail/events/Events.h"
class VkAPI;

class VkShader : public Shader, public IEventListener {
public:
	VkShader(Shaders::ShaderSettings settings);
	~VkShader();

	virtual void bind(void* cmdList) const override;

	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

	virtual bool setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) override;
	void setCBufferVar(const std::string& name, const void* data, unsigned int size) override;
	bool trySetCBufferVar(const std::string& name, const void* data, unsigned int size) override;

	// Call this after each mesh/instance
	// Internally updates meshIndex used to place multiple instances in a single cbuffer
	void instanceFinished();
	void reserve(unsigned int meshIndexMax);

	bool onEvent(Event& event) override;

private:
	unsigned int getMeshIndex() const;
private:
	VkAPI* m_context;
};