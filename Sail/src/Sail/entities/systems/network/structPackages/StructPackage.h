#pragma once

#include <string>


enum STRUCTTYPES {
	TRANSFORM,
	ROTATION,
	TRANSFORM_ROTATION,
	
	COUNT
};

struct easyVector {
	float x, y, z;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(x, y, z);
	}
};
struct myStruct {
	easyVector trans;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(trans);
	}
};