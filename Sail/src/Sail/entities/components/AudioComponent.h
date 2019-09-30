#ifndef AUDIO_COMPONENT_H
#define AUDIO_COMPONENT_H

#include "Sail/entities/components/Component.h"
#include <string>
#include <stack>

namespace SoundType {
	enum SoundType{WALK, RUN, SHOOT, JUMP, COUNT};
}

class AudioComponent : public Component<AudioComponent>
{
public:
	AudioComponent();
	virtual ~AudioComponent();

	std::string m_soundEffects[SoundType::COUNT];
	int m_soundID[SoundType::COUNT];
	float m_soundEffectTimers[SoundType::COUNT];
	float m_soundEffectThresholds[SoundType::COUNT];
	bool m_isPlaying[SoundType::COUNT];
	bool m_playOnce[SoundType::COUNT];

	/*Searches the container for an element with k as key and returns an iterator
	to it if found, otherwise it returns an iterator to unordered_map::end
	(the element past the end of the container).*/

	std::list<std::pair<std::string, bool>> m_streamingRequests;  // Rename: m_toBeStreamed
	std::list<std::pair<std::string, int>> m_currentlyStreaming; // Rename: m_currentlyStreaming

	// This function is purely here to MAKE LIFE LESS DIFFICULT
	void defineSound(SoundType::SoundType type, std::string filename, float dtThreshold = 0.0f, bool playOnce = true);
};

#endif
