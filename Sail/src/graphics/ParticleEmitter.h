#pragma once

#include "shader/instanced/ParticleShader.h"
#include "Particle.h"

class ParticleEmitter {
public:
	enum Type {
		EXPLOSION,
		FIREBALL
	};
	ParticleEmitter(Type type, const DirectX::SimpleMath::Vector3& emitPos, const DirectX::SimpleMath::Vector3& velocityRndAdd, const DirectX::SimpleMath::Vector3& velocityVariety, float spawnsPerSecond, UINT maxParticles, float scale = 1.f,
		float lifetime = 10.f, const DirectX::SimpleMath::Vector4& color = DirectX::SimpleMath::Vector4::One, float gravityScale = 0.f, UINT initialSpawnCount = 0,
		bool useAdditiveBlending = false, bool singleUse = true);
	~ParticleEmitter();

	// Updates particles, returns true if singleUse emitter is finished
	bool update(float dt);
	void draw();
	UINT getParticleCount() const;
	void setCamera(const Camera* cam);

	void updateEmitPosition(const DirectX::SimpleMath::Vector3& emitPos);
	void updateColor(const DirectX::SimpleMath::Vector4& color);
	void updateSpawnsPerSecond(float spawnsPerSec);
	void updateGravityScale(float gravityScale);
	void updateVelocityVariety(const DirectX::SimpleMath::Vector3& velVar);
	void updateVelocityRndAdd(const DirectX::SimpleMath::Vector3& velRndAdd);
	const DirectX::SimpleMath::Vector3& getEmitterPosition() const;

	// try this
	void spawnBeamParticles(const DirectX::SimpleMath::Vector3& startPos, const DirectX::SimpleMath::Vector3& endPos, float step, float minDuration, float maxDuration);

	// probably not this
	/*void setAsBeam();
	void setNextBeamPos(const DirectX::SimpleMath::Vector3& nextPos);*/


private:
	struct Compare {
		Compare(const ParticleEmitter& c) : myClass(c) {}
		bool operator()(const ParticleShader::InstanceData& i, const ParticleShader::InstanceData& j) {
			if (!myClass.m_cam)
				return false;
			return (DirectX::SimpleMath::Vector3::DistanceSquared(i.position, myClass.m_cam->getPosition()) > DirectX::SimpleMath::Vector3::DistanceSquared(j.position, myClass.m_cam->getPosition()));
		}
		const ParticleEmitter& myClass;
	};

	void insertionSort();
	void addParticle(Particle& particle);

private:
	std::vector<ParticleShader::InstanceData> m_instanceData;
	std::unique_ptr<Model> m_instancedModel;
	std::vector<Particle> m_particles;
	ParticleShader* m_shader;
	const Camera* m_cam;
	UINT m_spritesPerRow;
	UINT m_spritesPerColumn;
	bool m_useAdditiveBlending;
	bool m_singleUse;
	float m_spawnTimer;

	// dont think i need these
	// Beam settings
	/*bool m_isBeam;
	DirectX::SimpleMath::Vector3 m_beamStartPos;
	DirectX::SimpleMath::Vector3 m_beamEndPos;
	float m_beamMinDuration;
	float m_beamMaxDuration;*/

	// Particle settings
	DirectX::SimpleMath::Vector3 m_emitPosition;
	DirectX::SimpleMath::Vector3 m_velocityVariety;
	DirectX::SimpleMath::Vector3 m_velocityRndAdd;
	float m_spawnsPerSecond;
	float m_scale;
	float m_lifetime;
	float m_gravityScale;
	DirectX::SimpleMath::Vector4 m_color;

};