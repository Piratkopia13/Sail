#pragma once

// TODO: remove includes
#include "Sail/entities/components/Component.h"
//#include <x3daudio.h>
#include <string>
#include <stack>

namespace SoundType {
	enum SoundType{WALK, RUN, SHOOT, JUMP, LANDING, COUNT};
}

class AudioComponent : public Component<AudioComponent>
{
public:
	AudioComponent();
	virtual ~AudioComponent();

	//X3DAUDIO_LISTENER listener;

	std::string m_soundEffects[SoundType::COUNT];
	int m_soundID[SoundType::COUNT];
	float m_soundEffectTimers[SoundType::COUNT];
	float m_soundEffectThresholds[SoundType::COUNT];
	bool m_isPlaying[SoundType::COUNT];
	bool m_playOnce[SoundType::COUNT];

	// • string = filename
	// • bool = TRUE if START-request, FALSE if STOP-request
	std::list<std::pair<std::string, bool>> m_streamingRequests;
	// • string = filename
	// • int = ID of playing streaming; needed for STOPPING the streamed sound
	std::list<std::pair<std::string, int>> m_currentlyStreaming;

	// This function is purely here to MAKE LIFE LESS DIFFICULT
	void defineSound(SoundType::SoundType type, std::string filename, float dtThreshold = 0.0f, bool playOnce = true);
};

