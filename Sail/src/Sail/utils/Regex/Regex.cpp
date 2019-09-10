#include "pch.h"
#include "Regex.h"

int CharClass::match(char const* text) {
	return contents.find(*text) == std::string::npos ? -1 : 1;
}

CharClass::CharClass(std::string c) 
	: contents(c)
{

}

Star::Star(Regex* operand)
	: operand(operand)
{

}

Star::~Star() {
	//if (operand) 
		//delete operand;
}

int Star::match(char const* text) {
	int n, consumed = 0;
	while ((n = operand->match(text)) > 0) {
		consumed += n;
		text += n;
	}
	return consumed;
}

Seq::Seq(std::initializer_list<Regex*> cells)
	: cells(cells) {

}

Seq::~Seq() {
	//for (int i = 0; i < cells.size(); i++) 
	//	if (cells[i]) { 
	//		delete cells[i]; 
	//		cells[i] = nullptr; 
	//	} 
	//cells.clear(); 
}

int Seq::match(char const* text) {
	int chars, consumed = 0;
	for (auto c : cells) {
		if ((chars = c->match(text)) < 0)
			return -1;
		consumed += chars;
		text += chars;
	}
	return consumed;
}

Choice::Choice(std::initializer_list<Regex*> cells)
	: cells(cells) {

}

Choice::~Choice() {
	//for (int i = 0; i < cells.size(); i++) 
	//	if (cells[i]) if (cells[i]) { 
	//		delete cells[i]; 
	//		cells[i] = nullptr; 
	//	}
	//cells.clear();
}

int Choice::match(char const* text) {
	int chars;
	for (auto c : cells) {
		if ((chars = c->match(text)) > 0)
			return chars;
	}
	return -1;
}