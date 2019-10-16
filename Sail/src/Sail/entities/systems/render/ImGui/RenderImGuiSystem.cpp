#include "pch.h"
#include "imgui.h"
#include "RenderImGuiSystem.h"
#include "../../../ECS.h"
#include "../../Graphics/AnimationSystem.h"
#include "../../../../graphics/geometry/Animation.h"
#include "../../../components/NoEntityComponent.h"
#include "../../../components/AnimationComponent.h"

RenderImGuiSystem::RenderImGuiSystem() {
	registerComponent<NoEntityComponent>(true, true, true);
}

RenderImGuiSystem::~RenderImGuiSystem() {
}

void RenderImGuiSystem::renderImGuiAnimationSettings() {
	ImGui::Begin("Animation settings");
	bool interpolate = ECS::Instance()->getSystem<AnimationSystem>()->getInterpolation();
	ImGui::Checkbox("enable animation interpolation", &interpolate);
	ECS::Instance()->getSystem<AnimationSystem>()->setInterpolation(interpolate);
	ImGui::Separator();

	const std::vector<Entity*>& e = ECS::Instance()->getSystem<AnimationSystem>()->getEntities();

	if (ImGui::CollapsingHeader("Animated Objects")) {
		for (unsigned int i = 0; i < e.size(); i++) {
			if (ImGui::TreeNode(e[i]->getName().c_str())) {
				AnimationComponent* animationC = e[i]->getComponent<AnimationComponent>();
				ImGui::Text("Animation: %s", animationC->currentAnimation->getName().c_str());
				ImGui::Checkbox("Update on GPU", &animationC->computeUpdate);
				if (ImGui::SliderFloat("Animation Speed", &animationC->animationSpeed, 0.0f, 3.0f)) {

				}
				AnimationStack* stack = animationC->getAnimationStack();
				float w = animationC->animationW;
				ImGui::SliderFloat("weight", &w, 0.0f, 1.0f);
				ImGui::Text("AnimationStack");
				for (unsigned int animationTrack = 0; animationTrack < stack->getAnimationCount(); animationTrack++) {
					float time = -1;
					if (animationC->currentAnimation == stack->getAnimation(animationTrack)) {
						time = animationC->animationTime;
					}
					if (animationC->nextAnimation == stack->getAnimation(animationTrack)) {
						if (time > -1) {
							float time2 = animationC->transitions.front().transpiredTime;
							ImGui::SliderFloat(std::string("CurrentTime: " + std::to_string(animationTrack) + "T").c_str(), &time2, 0.0f, stack->getAnimation(animationTrack)->getMaxAnimationTime());

						}
						else {
							time = animationC->transitions.front().transpiredTime;

						}
					}
					if (time == -1) {
						time = 0;

					}
					ImGui::SliderFloat(std::string("CurrentTime: " + std::to_string(animationTrack)).c_str(), &time, 0.0f, stack->getAnimation(animationTrack)->getMaxAnimationTime());
					if (animationC->currentAnimation == stack->getAnimation(animationTrack)) {
						animationC->animationTime = time;
					}
				}

				static float transitionTime = 0.4f;
				static bool transitionWait = false;
				ImGui::Checkbox("transition wait", &transitionWait);
				ImGui::SameLine();
				if (ImGui::SliderFloat("Transition Time", &transitionTime, 0.0f, 1.0f)) {

				}
				for (unsigned int animationIndex = 0; animationIndex < stack->getAnimationCount(); animationIndex++) {




					if (ImGui::Button(std::string("Switch to " + stack->getAnimation(animationIndex)->getName()).c_str())) {
						animationC->transitions.emplace(stack->getAnimation(animationIndex), transitionTime, transitionWait);
					}
					ImGui::Separator();
				}

				ImGui::TreePop();
			}
		}
	}

	ImGui::End();
}
