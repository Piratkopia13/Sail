#include "pch.h"
#include "CandleReignitionSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

#include "Sail/events/EventDispatcher.h"
#include "Sail/events/types/IgniteCandleEvent.h"

CandleReignitionSystem::CandleReignitionSystem() {
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<LightComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(false, true, false);

	EventDispatcher::Instance().subscribe(Event::Type::IGNITE_CANDLE, this);
}

CandleReignitionSystem::~CandleReignitionSystem() {
	EventDispatcher::Instance().unsubscribe(Event::Type::IGNITE_CANDLE, this);
}

void CandleReignitionSystem::update(float dt) {
	for (auto& e : entities) {
		auto candle = e->getComponent<CandleComponent>();
		if (!candle->isLit) {
			if (candle->downTime >= m_candleForceRespawnTimer || candle->userReignition) {
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

bool CandleReignitionSystem::onEvent(const Event& event) {
	auto onIgniteCandle = [&](const IgniteCandleEvent& e) {
		Entity* candle = nullptr;

		// Find the candle whose parent has the correct ID
		for (auto candleEntity : entities) {
			if (auto parentEntity = candleEntity->getParent(); parentEntity) {
				if (parentEntity->getComponent<NetworkReceiverComponent>()->m_id == e.netCompID) {
					candle = candleEntity;
					break;
				}
			}
		}

		// candle exists => player exists (only need to check candle)
		if (!candle) {
			Logger::Warning("igniteCandle called but no matching entity found");
			return;
		}

		auto candleComp = candle->getComponent<CandleComponent>();
		if (!candleComp->isLit) {
			candleComp->health = MAX_HEALTH;
			candleComp->respawns++;
			candleComp->downTime = 0.f;
			candleComp->isLit = true;
			candleComp->userReignition = false;
			candleComp->invincibleTimer = 1.5f;
		}
	};

	switch (event.type) {
	case Event::Type::IGNITE_CANDLE: onIgniteCandle((const IgniteCandleEvent&)event); break;
	default: break;
	}

	return true;
}
