#pragma once

#include "Sail/graphics/shader/Shader.h"

class GenerateMipsComputeShader : public Shader {
public:
	class Input : public Shader::ComputeShaderInput {
	public:
		Texture* inputTexture;
	};
	class Output : public Shader::ComputeShaderOutput {
	public:
		RenderableTexture* outputTexture;
	};

public:
	GenerateMipsComputeShader();
	~GenerateMipsComputeShader();

	virtual const Shader::ComputeSettings* getComputeSettings() const override;
	virtual std::pair<std::string, void*> getComputeInputForIndex(Shader::ComputeShaderInput& input, unsigned int index) override;
	virtual RenderableTexture* getComputeOutputForIndex(Shader::ComputeShaderOutput& output, unsigned int index) override;
	virtual Shader::ComputeShaderOutput* getComputeOutput() override;

private:
	bool m_clippingPlaneHasChanged;
	Shader::ComputeSettings m_settings;
	Output m_output;

	struct alignas(16) GenerateMipsCB {
		uint32_t SrcMipLevel;           // Texture level of source mip
		uint32_t NumMipLevels;          // Number of OutMips to write: [1-4]
		uint32_t SrcDimension;          // Width and height of the source texture are even or odd.
		uint32_t IsSRGB;                // Must apply gamma correction to sRGB textures.
		glm::vec2 TexelSize;			// 1.0 / OutMip1.Dimensions
	};

};
