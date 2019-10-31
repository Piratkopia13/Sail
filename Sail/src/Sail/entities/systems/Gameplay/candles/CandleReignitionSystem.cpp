#include "pch.h"
#include "CandleReignitionSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

CandleReignitionSystem::CandleReignitionSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<LightComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(false, true, false);
}

CandleReignitionSystem::~CandleReignitionSystem() {}

void CandleReignitionSystem::update(float dt) {
	for (auto& e : entities) {
		auto candle = e->getComponent<CandleComponent>();
		if (!candle->isLit) {
			if (candle->downTime >= m_candleForceRespawnTimer || candle->userActivation) {
				NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
					Netcode::MessageType::IGNITE_CANDLE,
					SAIL_NEW Netcode::MessageIgniteCandle{
						e->getParent()->getComponent<NetworkReceiverComponent>()->m_id,
					},
					true
					);
			}

			candle->downTime += dt;
		}
	}
}
