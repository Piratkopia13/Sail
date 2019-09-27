#pragma once

#include "Sail.h"

#include "Network/NWrapper.h"
#include "../BaseComponentSystem.h"

class NetworkSerializedPackageEvent;

class NetworkSystem : public BaseComponentSystem {
public:
	NetworkSystem();
	virtual ~NetworkSystem() {}

	virtual void update(float dt) = 0;
	// Constructor initializes pNWrapper
	void initialize(Entity* playerEntity);

	virtual bool onSerializedPackageRecieved(NetworkSerializedPackageEvent& event) = 0;

protected:
	Entity* m_playerEntity = nullptr;
	NWrapper* m_network = nullptr;

	/*
		Temporarily used instead of glm::vector as all serialized data needs the
		'serialize' function, which glm::vector does not have unless hoops are
		jumped through. 
	*/
	struct easyVector {	
		float x, y, z;

		template<class Archive>
		void serialize(Archive& ar) {
			ar(x, y, z);
		}
	}; 

	struct TranslationStruct {
		easyVector trans;

		template<class Archive>
		void serialize(Archive& ar) {
			ar(trans);
		}
	};

	struct TransformPackage {
		easyVector m_translation;
		easyVector m_rotation;
		easyVector m_rotationQuat;
		easyVector m_scale;
		easyVector m_forward;
		easyVector m_right;
		easyVector m_up;

		template<class Archive>
		void serialize(Archive& ar) {
			ar(
				m_translation,
				m_rotation,
				m_rotationQuat,
				m_scale,
				m_forward,
				m_right,
				m_up
			);
		}
	};
};