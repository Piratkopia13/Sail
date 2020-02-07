#pragma once

#include "Sail/api/gui/SailGuiWindow.h"
#include "Sail/entities/Entity.h"

class EntitiesGui : public SailGuiWindow {
public:
    EntitiesGui();
    void render(std::vector<Entity::SPtr>& entities);

};