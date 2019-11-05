#pragma once
#include <vector>
#include "Event.h"

/* Notes */
// =======================================================================================================================
// | This event dispatcher should never be declared manually, since it's a singleton                                     |
// | Preferably, include and use it only in .cpp-files                                                                   |
// -----------------------------------------------------------------------------------------------------------------------
// | When creating an event, the event type needs to be set and the value reference "needs" to be cast to a void pointer |
// | When emitting an event, create and send a new event in one go                                                       |
// -----------------------------------------------------------------------------------------------------------------------
// | I.e. EventDispatcher::Get()->Emit(Event(EventType::CREATED_ENTITY, (void*)&playerPos));                             |
// =========================================================================================================================================
// | The class receiving events needs to inherit from EventReceiver and override void onEvent(const Event& e)                         |
// | In onEvent(const Event& e), the event type needs to be checked, preferably with a switch statement                               |
// | If the type matches with one of the cases, the value of the event needs to be statically cast to that data type before it can be used |
// -----------------------------------------------------------------------------------------------------------------------------------------
// | I.e.                                                                                                                                  |
// | void onEvent(const Event& e) {                                                                                                   |
// |     switch(e.type) {                                                                                                                  |
// |     case CREATED_POSITION:                                                                                                            |
// |         Vector3* pos = static_cast<Vector3*>(e.value);                                                                                |
// |         // Use pos here                                                                                                               |
// |         break;                                                                                                                        |
// |     }                                                                                                                                 |
// | }                                                                                                                                     |
// =========================================================================================================================================
// | Receivers can only receive events of a type they're subscribed to                                                     |
// | Receivers can be subscribed to an event without having a case for it, or vice versa, but preferrably not              |
// | One receiver can be subscribed to multiple event types, but will only become a subscriber once per type               |
// -------------------------------------------------------------------------------------------------------------------------
// | A receiver can either subscribe itself, or have another class do it. The same applies with unsubscribing              |
// -------------------------------------------------------------------------------------------------------------------------
// | I.e. EventDispatcher::Get()->Subscribe(EventType::CREATED_POSITION, &positionManager);                                |
// =========================================================================================================================
// | It is recommended, but not always necessary, for a class to unsubscribe itself from all events when it's destroyed |
// | Otherwise the dispatcher might attempt to call onEvent() on a destroyed object                                |
// ======================================================================================================================

class EventReceiver;

class EventDispatcher final {
public:
	static EventDispatcher& Instance() {
		static EventDispatcher ed;
		return ed;
	}

	~EventDispatcher() = default;

	void emit(const Event& e);
	void subscribe(const Event::Type& type, EventReceiver* subscriber);
	void unsubscribe(const Event::Type& type, EventReceiver* subscriber);

private:
	EventDispatcher() = default;
	bool subscribed(const Event::Type& type, EventReceiver* subscriber);

private:
	std::vector<EventReceiver*> m_subscribers[static_cast<size_t>(Event::Type::NR_OF_EVENTS)];
};
