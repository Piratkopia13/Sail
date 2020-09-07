#include "pch.h"
#include "SVkShader.h"
#include "../SVkAPI.h"
#include "Sail/Application.h"
//#include "VkConstantBuffer.h"
//#include "../resources/DescriptorHeap.h"
#include "../resources/VkTexture.h"
//#include "../resources/VkRenderableTexture.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW SVkShader(settings);
	else
		return new (allocAddr) SVkShader(settings);
}

SVkShader::SVkShader(Shaders::ShaderSettings settings)
	: Shader(settings)
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_context = Application::getInstance()->getAPI<SVkAPI>();

	compile();
}

SVkShader::~SVkShader() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);
}

void* SVkShader::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {
	
	std::string path = filepath;

#ifdef _WINDOWS_
	// Try to compile hlsl into spir-v
	if (strcmp(filepath.c_str() + filepath.size() - 4, "hlsl") == 0) {
		Logger::Log("Compiling " + filepath + " into spir-v...");

		std::string stageStr = (shaderType == ShaderComponent::BIND_SHADER::VS) ? "VS" : (shaderType == ShaderComponent::BIND_SHADER::PS)
			? "PS" : (shaderType == ShaderComponent::BIND_SHADER::HS) ? "HS" : (shaderType == ShaderComponent::BIND_SHADER::GS)
			? "GS" : (shaderType == ShaderComponent::BIND_SHADER::DS) ? "DS" : "CS";

		std::stringstream cmd;
		cmd << "..\\shaderconductor\\ShaderConductorCmd.exe";
		cmd << " -E " << stageStr << "Main"; // Entrypoint: VSMain, PSMain, etc.
		cmd << " -I " << filepath;
		std::transform(stageStr.begin(), stageStr.end(), stageStr.begin(), ::tolower); // The next -S argument requires lowercase
		std::string outputPath = filepath;
		outputPath.replace(outputPath.end() - 5, outputPath.end(), "\0");
		outputPath += "-"+stageStr+".spv";
		cmd << " -O " << outputPath;
		cmd << " -S " << stageStr; // vs, ps, etc.
		cmd << " -T spirv";

		system(cmd.str().c_str());

		// The outputed spir-v file is the one we want to read
		path = outputPath;
	}

#else
#endif

	auto shaderCode = Utils::readFileBinary(path);
	/*auto shaderCode = Utils::readFileBinary("res/shaders/vulkan/vert.spv");
	if (shaderType == ShaderComponent::BIND_SHADER::PS)
		shaderCode = Utils::readFileBinary("res/shaders/vulkan/frag.spv");*/

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule* shaderModule = new VkShaderModule;
	if (vkCreateShaderModule(m_context->getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		Logger::Error("Failed to create shader module!");
	}

	return shaderModule;
}

bool SVkShader::onEvent(Event& event) {
	assert(false);
	return true;
}

void SVkShader::bind(void* cmdList) const {
	bindInternal(0U, cmdList);
}

bool SVkShader::setTexture(const std::string& name, Texture* texture, void* cmdList) {
	assert(false);
	return true;
}

void SVkShader::setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) {
	assert(false);
}

void SVkShader::setCBufferVar(const std::string& name, const void* data, unsigned int size) {
	setCBufferVarInternal(name, data, size, 0U);
}

bool SVkShader::trySetCBufferVar(const std::string& name, const void* data, unsigned int size) {
	return trySetCBufferVarInternal(name, data, size, 0U);
}