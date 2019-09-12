#pragma once

#include <string>
#include <vector>

class Regex {
public:
	virtual ~Regex() {};
	virtual int match(char const *) = 0;

};

class CharClass : public Regex {
public:
	CharClass(std::string c);
	int match(char const* text) override;
public:
	std::string contents;
};

class Star : public Regex {
public:
	Star(Regex* operand);
	~Star();
	int match(char const* text) override;
public:
	Regex* operand;
};

class Seq : public Regex {
public:
	Seq(std::initializer_list<Regex*> cells);
	~Seq();
	int match(char const* text) override;
public:
	std::vector<Regex*> cells;
};

class Choice : public Regex {
public:
	Choice(std::initializer_list<Regex*> cells);
	~Choice();
	int match(char const* text) override;
public:
	std::vector<Regex*> cells;
};


namespace Reg {
	static CharClass Digit("0123456789");
	static CharClass NonZero("123456789");
	static CharClass zeroChar("0");
	static CharClass xChar("x");
	static CharClass Dot(".");
	static CharClass Minus("-");
	static CharClass Equals("=");
	static CharClass QuotationMark("\"");
	static CharClass OpenBracket("[");
	static CharClass CloseBracket("]");
	static CharClass OpenCurlyBracket("{");
	static CharClass CloseCurlyBracket("}");
	static CharClass HexDigit("0123456789abcdefABCDEF");
	static CharClass Alphanumeric("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	static CharClass TextCharacter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

	static Star DigitStar(&Digit);
	static Star MinusStar(&Minus);
	static Star DotStar(&Dot);
	static Star HexStar(&HexDigit);

	static Seq Hex({ &zeroChar, &xChar, &HexDigit, &HexStar});
	static Seq Number({ &MinusStar, &Digit, &DigitStar });
	static Seq DecimalNumber({ &MinusStar, &Digit, &DigitStar, &DotStar, &DigitStar });

	static CharClass Whitespace(" ");
	static Star WhitespaceStar(&Whitespace);

	static Star AlphanumericStar(&Alphanumeric);
	static Seq AlphanumericString({ &Alphanumeric, &AlphanumericStar });
	static Star TextCharacterStar(&TextCharacter);
	static Seq TextString({ &TextCharacter, &TextCharacterStar });


	


}