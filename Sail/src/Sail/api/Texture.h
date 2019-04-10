#pragma once

class Texture {
public:
	static Texture* create(const std::string& filename);
	Texture() {}
	virtual ~Texture() {}

};