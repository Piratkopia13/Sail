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
