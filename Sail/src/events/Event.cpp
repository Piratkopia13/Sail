#include "Event.h"

Event::Event(Type type)
	: m_type(type)
{

}

Event::~Event() {

}

Event::Type Event::getType() const {
	return m_type;
}
