#pragma once
#include "../../BaseComponentSystem.h"
#include "../../..//components/PowerUp/PowerUpComponent.h"
#include "Sail/events/EventReceiver.h"
#include "Sail/netcode/NetcodeTypes.h"
#include "Sail/events/Events.h"

class PowerUpCollectibleSystem final : public BaseComponentSystem, public EventReceiver {
public:
	PowerUpCollectibleSystem();
	~PowerUpCollectibleSystem();
	void init(std::vector<Entity*>* playerList);
	void update(float dt) override;
	void spawnSingleUsePowerUp(const PowerUps powerUp, const float time, glm::vec3 pos, Entity* parent = nullptr);
	void spawnPowerUps(int amount);
#ifdef DEVELOPMENT
	unsigned int getByteSize() const override;
	void imguiPrint(Entity** selectedEntity = nullptr) override;
#endif
private:
	void spawnPowerUp(glm::vec3 pos, int powerUp, float time, float respawntime, Netcode::ComponentID compID = 0);
	void updateSpawns(const float dt);

	void onDestroyPowerUp(const DestroyPowerUp& e);
private:
	struct ReSpawn {
		float time;
		PowerUps powerUp;
		glm::vec3 pos;
	};
	float m_collectDistance;

	std::vector<Entity*>* m_playerList;
	std::list<ReSpawn> m_respawns;

	std::vector<float> m_distances;

	// Inherited via EventReceiver
	virtual bool onEvent(const Event& e) override;
};