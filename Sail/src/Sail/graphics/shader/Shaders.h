#pragma once
#include <string>
#include "../material/Material.h"
#include "Sail/api/GraphicsAPI.h"
#include "Sail/api/shader/PipelineStateObject.h"

namespace Shaders {
	enum ShaderIdentifier {
		PBRMaterialShader = 0,
		PhongMaterialShader,
		OutlineShader,
		CubemapShader,
		NUM_GRAPHICS_SHADERS,
		GenerateMipsComputeShader,
		NUM_TOTAL_SHADERS
	};
	static const char* shaderNames[]{ "PBRMaterialShader", "PhongMaterialShader", "OutlineMaterialShader", "CubemapShader", "-", "GenerateMipsComputeShader" };
	
	struct ComputeShaderSettings {
		float threadGroupXScale = 1.0f;
		float threadGroupYScale = 1.0f;
		float threadGroupZScale = 1.0f;
	};
	struct ShaderSettings {
		std::string filename;
		Material::Type materialType;
		PipelineStateObject::PSOSettings defaultPSOSettings;
		ComputeShaderSettings computeShaderSettings;
		ShaderIdentifier identifier; // only used in gui
	};
}