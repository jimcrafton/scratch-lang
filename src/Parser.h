#ifndef _PARSER_H__
#define _PARSER_H__


#include "Lexer.h"

enum ParseNodeType {
	UNKNOWN = 0,
	SCALAR,
	MODULE
};

template <int X>
class ParseNode {
public:
	ParseNodeType nodeType;

	ParseNode() : nodeType(X) {}
};

class ScalarNode : public ParseNode<SCALAR> {

};

class IntegerNode : public ScalarNode {

};

class DecimalNode : public ScalarNode {

};


class StringNode : public ScalarNode {

};


class BooleanNode : public ScalarNode {

};

class ModuleNode : public ParseNode<MODULE> {
public:

};




class Parser {
public:
	bool parseModule(const Lexer& lexer) {

		const Token& t = lexer.currentContext()->tokens[0];
		if (t.type == Token::IDENTIFIER && t.text == "module") {

		}

		return false;
	}

	bool parse(const Lexer& lexer) {
		
		if (!parseModule(lexer)) {
			return false;
		}

		return true;
	}
};


#endif //_PARSER_H__



