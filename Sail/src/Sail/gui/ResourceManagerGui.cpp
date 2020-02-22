#include "pch.h"
#include "ResourceManagerGui.h"
#include "imgui.h"
#include "../Application.h"

ResourceManagerGui::ResourceManagerGui() { }

void ResourceManagerGui::render() {
	newFrame();
	auto& resman = Application::getInstance()->getResourceManager();

	if (ImGui::Begin("Resource tracker")) {
		enableColumns(190.f);

		addProperty("Texture data in RAM:", [&resman] {
			ImGui::Text("%u textures, totaling %umb", resman.getTextureDataCount(), resman.getTextureDataSize());
		});
		addProperty("FBX model count:", [&resman] {
			ImGui::Text("%u", resman.getFBXModelCount());
		});
		addProperty("Shader count:", [&resman] {
			ImGui::Text("%u", resman.getShaderCount());
		});
		addProperty("PipelineStateObject count:", [&resman] {
			ImGui::Text("%u", resman.getPSOCount());
		});

		disableColumns();

		ImGui::Text("VRAM usage: %u / %u MB", Application::getInstance()->getAPI()->getMemoryUsage(), Application::getInstance()->getAPI()->getMemoryBudget());
	}
	ImGui::End();
}
