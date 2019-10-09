#pragma once

#include "Sail/entities/components/Component.h"
#include <string>
#include <stack>

namespace SoundType {
	enum SoundType{
		AMBIENT,
		WALK, 
		RUN, 
		SHOOT, 
		JUMP, 
		LANDING, 
		COUNT // Keep at the bottom so that COUNT == nr of sound types
	};
}

class AudioComponent : public Component<AudioComponent>
{
public:
	AudioComponent();
	virtual ~AudioComponent();


	// TODO: array of structs instead for better cache performance

	std::string m_soundEffects[SoundType::COUNT];
	int m_soundID[SoundType::COUNT];
	float m_soundEffectTimers[SoundType::COUNT];
	float m_soundEffectLengths[SoundType::COUNT];
	bool m_isPlaying[SoundType::COUNT];
	bool m_playOnce[SoundType::COUNT];
	bool m_isInitialized[SoundType::COUNT];
	glm::vec3 m_positionalOffset[SoundType::COUNT];
	float m_volume[SoundType::COUNT];

	// • string = filename
	// • bool = TRUE if START-request, FALSE if STOP-request
	std::list<std::pair<std::string, bool>> m_streamingRequests;
	// • string = filename
	// • int = ID of playing streaming; needed for STOPPING the streamed sound
	std::list<std::pair<std::string, int>> m_currentlyStreaming;


	// This function is purely here to MAKE LIFE LESS DIFFICULT
	void defineSound(
		SoundType::SoundType type,
		const std::string& filename,
		float soundLength,
		bool playOnce = true,
		float volume = 1.0f,
		const glm::vec3& positionOffset = { 0.f,0.f,0.f });
};

