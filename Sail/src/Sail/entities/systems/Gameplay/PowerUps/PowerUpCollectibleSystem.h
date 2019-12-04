#pragma once
#include "../../BaseComponentSystem.h"
#include "../../..//components/PowerUp/PowerUpComponent.h"

class PowerUpCollectibleSystem final : public BaseComponentSystem {
public:
	PowerUpCollectibleSystem();
	~PowerUpCollectibleSystem();
	void init(std::vector<Entity*>* playerList);
	void setSpawnPoints(std::vector<glm::vec3>& points);
	void setRespawnTime(const float time);
	void setDuration(const float time);
	void update(float dt) override;
	void spawnSingleUsePowerUp(const PowerUps powerUp, const float time, glm::vec3 pos, Entity* parent = nullptr);
	void spawnPowerUps(int amount = -1);
#ifdef DEVELOPMENT
	unsigned int getByteSize() const override;
	void imguiPrint(Entity** selectedEntity = nullptr) override;
#endif
private:
	void spawnPowerUp(glm::vec3 pos, int powerUp, float time, float respawntime);
	void updateSpawns(const float dt);
private:
	struct ReSpawn {
		float time;
		PowerUps powerUp;
		glm::vec3 pos;
	};
	float m_collectDistance;
	float m_respawnTime;
	float m_duration;


	std::vector<Entity*>* m_playerList;
	std::list<ReSpawn> m_respawns;
	std::list<glm::vec3> m_spawnPoints;
	std::vector<float> m_distances;
};