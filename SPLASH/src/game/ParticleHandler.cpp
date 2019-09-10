#include "ParticleHandler.h"

ParticleHandler::ParticleHandler(const Camera* cam)
	: m_cam(cam)
{ }

ParticleHandler::~ParticleHandler() { }

void ParticleHandler::addEmitter(std::shared_ptr<ParticleEmitter>& emitter) {
	m_emitters.push_back(emitter);
	m_emitters.back()->setCamera(m_cam);
}

void ParticleHandler::removeEmitter(std::shared_ptr<ParticleEmitter>& emitter) {
	m_emitters.erase(std::remove(m_emitters.begin(), m_emitters.end(), emitter), m_emitters.end());
}

void ParticleHandler::update(float dt) {
	auto it = m_emitters.begin();
	while (it != m_emitters.end()) {
		bool remove = (*it)->update(dt);

		if (remove)
			it = m_emitters.erase(it);
		else
			++it;
	}

	/*double time = m_timer.getFrameTime();
	std::cout << "Particle updates took: " << time * 1000.f << "ms" << std::endl << std::endl;*/
}

void ParticleHandler::draw() {
	for (auto& emitter : m_emitters)
		emitter->draw();
}

UINT ParticleHandler::getParticleCount() const {
	UINT count = 0;
	for (auto& emitter : m_emitters)
		count += emitter->getParticleCount();
	return count;
}

UINT ParticleHandler::getEmitterCount() const {
	return m_emitters.size();
}
