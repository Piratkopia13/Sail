#pragma once

#include "Sail/entities/components/Component.h"
#include "../Sail/src/Sail/entities/systems/Audio/AudioData.h"
#include <string>
#include <stack>
#include <typeinfo>

class AudioComponent : public Component<AudioComponent>
{
public:
	AudioComponent();
	virtual ~AudioComponent();

	Audio::SoundInfo_General m_sounds[Audio::SoundType::COUNT];

	// An 'easy-mode' helper function for starting/stopping a streamed sound
	void streamSoundRequest_HELPERFUNC(std::string filename, bool startTrue_stopFalse, float volume, bool isPositionalAudio, bool isLooping);
	void streamSetVolume_HELPERFUNC(std::string filename, float volume);

	// VARIABLE DEFINITIONS/CLARIFICATIONS
		// • string = filename
		// • bool = TRUE if START-request, FALSE if STOP-request
	std::list<std::pair<std::string, Audio::StreamRequestInfo>> m_streamingRequests;
	// VARIABLE DEFINITIONS/CLARIFICATIONS
		// • string = filename
		// • int = ID of playing streaming; needed for STOPPING the streamed sound
	std::list<std::pair<std::string, Audio::StreamRequestInfo>> m_currentlyStreaming;

#ifdef DEVELOPMENT
	void imguiRender(Entity** selected) {
		ImGui::Text("Currently Playing Sounds");
		ImGui::Indent(10.0f);
		int index = 0;
		for (const auto& sound : m_sounds) {
			if (sound.isPlaying) {
				Audio::SoundType type = static_cast<Audio::SoundType>(index);
				ImGui::Text(std::to_string(type).c_str());
			}
			index++;
		}
		ImGui::Unindent(10.0f);
		ImGui::Text("StreamRequests");
		ImGui::Indent(10.0f);
		for (const auto& sound : m_streamingRequests) {
			ImGui::Text(std::string(sound.first + "(" + std::to_string(sound.second.streamIndex) + ":" + std::to_string(sound.second.isPositionalAudio) + ")").c_str());
		}
		ImGui::Unindent(10.0f);
		ImGui::Text("Streams");
		ImGui::Indent(10.0f);
		for (const auto& sound : m_currentlyStreaming) {
			ImGui::Text(std::string(sound.first + "(" + std::to_string(sound.second.streamIndex) + ":" + std::to_string(sound.second.isPositionalAudio) + ")").c_str());
		}
		ImGui::Unindent(10.0f);
	}
#endif


};
