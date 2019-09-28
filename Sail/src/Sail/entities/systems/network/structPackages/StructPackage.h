#pragma once

#include <string>


enum STRUCTTYPES {
	TRANSFORM,
	
	COUNT
};


class StructPackage {
public:
	StructPackage() {}
	~StructPackage() {}

	virtual void parse(std::string data) = 0;

protected:
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

};


class TransformStruct : public StructPackage {
public:
	TransformStruct() {}
	~TransformStruct() {}

	void parse(std::string data) override {

	}

private:
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

	struct myStruct {
		easyVector trans;

		template<class Archive>
		void serialize(Archive& ar) {
			ar(trans);
		}
	};

};