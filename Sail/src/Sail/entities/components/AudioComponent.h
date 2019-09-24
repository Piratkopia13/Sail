#ifndef AUDIO_COMPONENT_H
#define AUDIO_COMPONENT_H

#include <string>

namespace SoundType {
	enum SoundType{WALK, RUN, SHOOT, COUNT};
}

#include "Sail/entities/components/Component.h"

class AudioComponent : public Component<AudioComponent>
{
public:
	AudioComponent();
	virtual ~AudioComponent();

private:
	std::string m_soundEffects[SoundType::COUNT];
	float m_soundEffectTimers[SoundType::COUNT];
	bool m_isPlaying[SoundType::COUNT];
};

#endif
