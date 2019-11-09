#include "pch.h"
#include "CandleReignitionSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

CandleReignitionSystem::CandleReignitionSystem() {
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<LightComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(true, true, false);
	registerComponent<LocalOwnerComponent>(true, false, false); // We only reignite our own candle
}

CandleReignitionSystem::~CandleReignitionSystem() {}

void CandleReignitionSystem::update(float dt) {
	for (auto& e : entities) {
		auto candle = e->getComponent<CandleComponent>();

		if (!candle->isLit) {
			candle->downTime += dt;

			if (candle->downTime >= m_candleForceRespawnTimer || candle->userReignition) {
				NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
					Netcode::MessageType::IGNITE_CANDLE,
					SAIL_NEW Netcode::MessageIgniteCandle{
						e->getComponent<NetworkSenderComponent>()->m_id
					},
					true
				);
			}
		}
	}
}
