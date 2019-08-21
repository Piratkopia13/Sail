#include "ParticleEmitter.h"
#include "geometry/factory/InstancedParticleModel.h"
#include <algorithm>
#include <math.h>

using namespace DirectX;
using namespace SimpleMath;

ParticleEmitter::ParticleEmitter(Type type, const Vector3& emitPos, const Vector3& velocityRndAdd, const Vector3& velocityVariety, 
	float spawnsPerSecond, UINT maxParticles, float scale, float lifetime, const Vector4& color, float gravityScale, UINT initialSpawnCount, 
	bool useAdditiveBlending, bool singleUse)
	: m_useAdditiveBlending(useAdditiveBlending)
	, m_velocityVariety(velocityVariety)
	, m_velocityRndAdd(velocityRndAdd)
	, m_spawnsPerSecond(spawnsPerSecond)
	, m_scale(scale)
	, m_color(color)
	, m_gravityScale(gravityScale)
	, m_lifetime(lifetime)
	, m_emitPosition(emitPos)
	, m_singleUse(singleUse)
	, m_spawnTimer(0.f)
	//, m_isBeam(false)
{

	auto* m_app = Application::getInstance();

	std::string particleSpritesheet;
	switch (type) {
	case ParticleEmitter::EXPLOSION:
		particleSpritesheet = "particles/explosion.tga";
		m_spritesPerColumn = 3;
		m_spritesPerRow = 3;
		break;
	case ParticleEmitter::FIREBALL:
		particleSpritesheet = "particles/fireball.tga";
		m_spritesPerColumn = 7;
		m_spritesPerRow = 7;
		break;
	default:
		particleSpritesheet = "particles/explosion.tga";
		m_spritesPerColumn = 3;
		m_spritesPerRow = 3;
		break;
	}

	// Load particle spritesheet if needed
	if (!m_app->getResourceManager().hasDXTexture(particleSpritesheet))
		m_app->getResourceManager().LoadDXTexture(particleSpritesheet);

	// Store a pointer to the shader used in rendering
	m_shader = &m_app->getResourceManager().getShaderSet<ParticleShader>();

	m_instancedModel = ModelFactory::InstancedParticleModel::Create(maxParticles, m_shader);
	m_instancedModel->getMaterial()->setDiffuseTexture(particleSpritesheet);

	// Resize vector to fit max particles
	m_instanceData.resize(maxParticles);

	// Spawn init particles
	for (UINT i = 0; i < initialSpawnCount; i++) {
		addParticle(Particle(m_emitPosition,
			Vector3((Utils::rnd() + m_velocityRndAdd.x) * m_velocityVariety.x, (Utils::rnd() + m_velocityRndAdd.y) * m_velocityVariety.y, (Utils::rnd() + m_velocityRndAdd.z) * m_velocityVariety.z),
			1.f, m_gravityScale, m_lifetime));
	}
}

ParticleEmitter::~ParticleEmitter() {
}

bool ParticleEmitter::update(float dt) {

	m_spawnTimer += dt;
	if (m_spawnTimer >= 1.f / m_spawnsPerSecond) {
		while (m_spawnTimer > 0.f) {
			// Spawn a new particle
			addParticle(Particle(m_emitPosition,
									Vector3((Utils::rnd() + m_velocityRndAdd.x) * m_velocityVariety.x, (Utils::rnd() + m_velocityRndAdd.y) * m_velocityVariety.y, (Utils::rnd() + m_velocityRndAdd.z) * m_velocityVariety.z),
									1.f, m_gravityScale, m_lifetime));
			m_spawnTimer -= 1.f / m_spawnsPerSecond;
		}
	}
	m_spawnTimer = 0.f;

	int i = 0;
	auto& it = m_particles.begin();
	while (it != m_particles.end()) {

		Particle& particle = *it;

		if (particle.isDead()) {
			it = m_particles.erase(it);
		} else {

			auto& data = m_instanceData[i];
			particle.update(dt);
			data.position = particle.getPosition();

			// Update texture offsets
			float lifePercent = particle.getLifePercentage();
			UINT numSprites = m_spritesPerColumn * m_spritesPerRow;
			UINT spriteNum = (UINT)floorf(lifePercent * (float)numSprites);
			spriteNum = (spriteNum == numSprites) ? numSprites - 1 : spriteNum;

			data.textureOffset1.x = fmod(spriteNum / (float)m_spritesPerRow, 1.f);
			data.textureOffset1.y = (spriteNum / m_spritesPerRow) / (float)m_spritesPerRow;

			(spriteNum < numSprites - 1) ? spriteNum++ : spriteNum = spriteNum;
			data.textureOffset2.x = fmod(spriteNum / (float)m_spritesPerRow, 1.f);
			data.textureOffset2.y = (spriteNum / m_spritesPerRow) / (float)m_spritesPerRow;
			data.color = m_color;
			data.color.w = 1.f - lifePercent;

			// Update blend factor
			data.blendFactor = fmod(lifePercent * numSprites, 1.f);
			i++;

			++it;
		}
	}

	if (m_singleUse && m_particles.size() == 0)
		return true;

	//#ifndef _DEBUG
		// Only sort if neccessary
	if (!m_useAdditiveBlending)
		// TOOD: try different sorting algorithms
		std::sort(m_instanceData.begin(), m_instanceData.begin() + m_particles.size(), Compare(*this));
	//insertionSort();
//#endif

	return false;
}

void ParticleEmitter::insertionSort() {
	unsigned int i, j;
	ParticleShader::InstanceData key;
	for (i = 1; i < m_instanceData.size(); i++) {
		key = m_instanceData[i];
		j = i - 1;

		while (j >= 0 && Vector3::DistanceSquared(m_instanceData[j].position, m_cam->getPosition()) < Vector3::DistanceSquared(key.position, m_cam->getPosition())) {
			m_instanceData[j + 1] = m_instanceData[j];
			j = j - 1;
		}
		m_instanceData[j + 1] = key;
	}
}

void ParticleEmitter::addParticle(Particle& particle) {
	if (m_particles.size() < m_instanceData.size())
		m_particles.push_back(particle);
}

void ParticleEmitter::updateEmitPosition(const DirectX::SimpleMath::Vector3& emitPos) {
	m_emitPosition = emitPos;
}

void ParticleEmitter::updateColor(const DirectX::SimpleMath::Vector4& color) {
	m_color = color;
}

void ParticleEmitter::updateSpawnsPerSecond(float spawnsPerSec) {
	m_spawnsPerSecond = spawnsPerSec;
}

void ParticleEmitter::updateGravityScale(float gravityScale) {
	m_gravityScale = gravityScale;
}

void ParticleEmitter::updateVelocityVariety(const DirectX::SimpleMath::Vector3& velVar) {
	m_velocityVariety = velVar;
}

void ParticleEmitter::updateVelocityRndAdd(const DirectX::SimpleMath::Vector3& velRndAdd) {
	m_velocityRndAdd = velRndAdd;
}

const DirectX::SimpleMath::Vector3& ParticleEmitter::getEmitterPosition() const {
	return m_emitPosition;
}

void ParticleEmitter::spawnBeamParticles(const DirectX::SimpleMath::Vector3& startPos, const DirectX::SimpleMath::Vector3& endPos, float step, float minDuration, float maxDuration) {
	float dst = Vector3::Distance(startPos, endPos);
	Vector3 dir = endPos - startPos;
	dir.Normalize();

	Vector3 rndAdd = -dir - Vector3(0.5f);

	// Step along the beam and spawn particles with different lifetimes
	for (float i = 0.f; i < dst; i += step) {
		addParticle(Particle(startPos + dir * i,
							 Vector3((Utils::rnd() + rndAdd.x) * m_velocityVariety.x, (Utils::rnd() + rndAdd.y) * m_velocityVariety.y, (Utils::rnd() + rndAdd.z) * m_velocityVariety.z),
							 1.f, m_gravityScale, minDuration + maxDuration * (i / dst)));
	}
}

void ParticleEmitter::draw() {
	m_shader->updateSpriteData(m_spritesPerRow, m_scale);
	m_shader->updateInstanceData(&m_instanceData[0], m_instanceData.size() * sizeof(m_instanceData[0]), m_instancedModel->getInstanceBuffer());
	Application::getInstance()->getAPI()->setDepthMask(GraphicsAPI::WRITE_MASK);
	if (m_useAdditiveBlending) {
		Application::getInstance()->getAPI()->setBlending(GraphicsAPI::ADDITIVE);
	} else
		Application::getInstance()->getAPI()->setBlending(GraphicsAPI::ALPHA);
	m_shader->draw(*m_instancedModel, true, m_particles.size());
	Application::getInstance()->getAPI()->setDepthMask(GraphicsAPI::NO_MASK);
}

UINT ParticleEmitter::getParticleCount() const {
	return m_particles.size();
}

void ParticleEmitter::setCamera(const Camera* cam) {
	m_cam = cam;
}
