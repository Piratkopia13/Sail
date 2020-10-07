#pragma once

// Define to compile ALL shaders using the DXIL compiler
// This is necessary for inline raytracing support but somewhat untested otherwise
//#define USE_DXIL_COMPILER


#include "Sail/api/shader/Shader.h"
#include <atomic>

#ifdef USE_DXIL_COMPILER
#include "DXILShaderCompiler.h"
#endif
#include "../resources/DX12Texture.h"

class DX12API;

class DX12Shader : public Shader {
public:
	DX12Shader(Shaders::ShaderSettings settings);
	~DX12Shader();

	virtual void bind(void* cmdList) const override;
	virtual void recompile() override;

	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

	virtual void updateDescriptorsAndMaterialIndices(Renderer::RenderCommandList renderCommands, const Environment& environment, const PipelineStateObject* pso, void* cmdList) override;

protected:
	virtual bool setConstantDerived(const std::string& name, const void* data, uint32_t size, ShaderComponent::BIND_SHADER bindShader, uint32_t byteOffset, void* cmdList) override;

private:
	DX12API* m_context;
#ifdef USE_DXIL_COMPILER
	DXILShaderCompiler m_dxilCompiler;
#endif

	// TODO: Abstract these out to Shader.h
	// Textures used while waiting for the proper textures to finish uploading to the GPU
	DX12Texture& m_missingTexture;
	DX12Texture& m_missingTextureCube;

};