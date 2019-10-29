#include "pch.h"
#include "CandleReignitionSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

CandleReignitionSystem::CandleReignitionSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<LightComponent>(true, true, true);
	registerComponent<NetworkReceiverComponent>(false, true, false);	
}

CandleReignitionSystem::~CandleReignitionSystem() {}

void CandleReignitionSystem::update(float dt) {
	for (auto& e : entities) {
		auto candle = e->getComponent<CandleComponent>();
		if (!candle->isLit) {
			candle->downTime += dt;

			if (candle->downTime >= m_candleForceRespawnTimer) {
				candle->downTime = 0.f;
				if (NWrapperSingleton::getInstance().isHost()) {
					NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
						Netcode::MessageType::IGNITE_CANDLE,
						SAIL_NEW Netcode::MessageIgniteCandle{
							e->getParent()->getComponent<NetworkReceiverComponent>()->m_id,
						},
						true
					);
				}
			}
		}
	}
}
