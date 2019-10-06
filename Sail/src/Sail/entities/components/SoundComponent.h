#pragma once

#include "Sail/entities/components/Component.h"
#include "API/Audio/OmnidirectionalSound.h"
//
//namespace SoundType {
//	enum SoundType { WALK, RUN, SHOOT, JUMP, LANDING, COUNT };
//}


class SoundComponent : public Component<SoundComponent>
{
public:
	SoundComponent() {}

	// TODO: clean up
	virtual ~SoundComponent() {}

	void addSound(std::string filename) {
		OmnidirectionalSound* sound = new OmnidirectionalSound(filename);
		if (SUCCEEDED(sound->Initialize())) {
			sound->_isQueued = true; // so that it plays
			sounds.emplace_back(sound);
		}
	}



	// TODO: some kind of trigger for the sound,
	// maybe an enum that's triggered per entity which reads from a component that saves
	// each entity's state (IS_SHOOTING, IS_ALIVE, IS_MOVING, etc.)
	/*struct SoundStruct {
		std::string filename = "";
		bool soundQueued = false;
		bool soundPlaying = false;
		OmnidirectionalSound sound;

		SoundStruct(const std::string& path) : sound(path), filename(path) {
		}
	};
	*/
	std::vector<OmnidirectionalSound*> sounds;
};

