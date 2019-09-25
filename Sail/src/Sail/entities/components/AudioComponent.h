#ifndef AUDIO_COMPONENT_H
#define AUDIO_COMPONENT_H

#include "Sail/entities/components/Component.h"
#include <string>

namespace SoundType {
	enum SoundType{WALK, RUN, SHOOT, JUMP, COUNT};
}

class AudioComponent : public Component<AudioComponent>
{
public:
	AudioComponent();
	virtual ~AudioComponent();

	std::string m_soundEffects[SoundType::COUNT];
	float m_soundEffectTimers[SoundType::COUNT];
	float m_soundEffectThresholds[SoundType::COUNT];
	bool m_isPlaying[SoundType::COUNT];
	bool m_isLooping[SoundType::COUNT];

	// This function is purely here to MAKE LIFE LESS DIFFICULT
	void defineSound(SoundType::SoundType type, std::string filename, float dtThreshold = 0.0f, bool isLooping = false);
};

#endif
