#ifndef _PARSER_H__
#define _PARSER_H__


#include "Lexer.h"




class ParseNode {
protected:

public:
	const ParseNode* parent = nullptr;

	ParseNode() {}

	virtual ~ParseNode() {}

	virtual bool hasChildren() const {
		return false;
	}

	virtual void clear() {
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {

	}

	virtual std::string printInfo() const { return std::string(); };
	
};


enum ParseErrorType {
	UNKNOWN_ERR = 0,
};


class Comment : public ParseNode {
public:
	std::string comments;
	virtual ~Comment() {}

	virtual bool hasChildren() const {
		return false;
	}
	virtual std::string printInfo() const {
		std::string result = "comment: " + comments;

		return result;
	};
};


class ParseNodeWithComments : public ParseNode {
public:
	std::vector<Comment*> comments;

	virtual ~ParseNodeWithComments() {
		for (auto& c : comments) {
			delete c;
		}
	}
	

	virtual bool hasChildren() const {
		return !comments.empty();
	}

	virtual void clear() {
		for (auto& c : comments) {
			delete c;
		}
		comments.clear();
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		children.insert(children.end(), comments.begin(), comments.end());
	}

};



class VariableNode : public ParseNodeWithComments {
public:
	std::string name;
	std::string type;

	virtual ~VariableNode() {
	}



	virtual std::string printInfo() const {
		std::string result = "var: " + name + " " + type;

		return result;
	};
};

class InstanceNode : public ParseNodeWithComments {
public:
	std::string name;

	virtual ~InstanceNode() {
	}



	virtual std::string printInfo() const {
		std::string result = "instance: " + name;

		return result;
	};
};




class LiteralNode : public ParseNodeWithComments {
public:

	enum Type {
		INTEGER_LITERAL,
		DECIMAL_LITERAL,
		HEXADECIMAL_LITERAL,
		BINARY_LITERAL,
		BOOLEAN_LITERAL,
		STRING_LITERAL,
	};

	std::string val;
	Type type;

	virtual ~LiteralNode() {
	}

	


	virtual std::string printInfo() const {
		std::string result = "literal(";
		
		switch (type) {
			case LiteralNode::INTEGER_LITERAL: result += "integer";break;
			case LiteralNode::DECIMAL_LITERAL: result += "decimal";break;
			case LiteralNode::HEXADECIMAL_LITERAL: result += "hex";break;
			case LiteralNode::BINARY_LITERAL: result += "bin";break;
			case LiteralNode::BOOLEAN_LITERAL: result += "bool";break;
			case LiteralNode::STRING_LITERAL: result += "string";break;
			default:result += "ERR!";break;
		}

		result += "): " + val;

		return result;
	};
};

class ArrayLiteralNode : public ParseNodeWithComments {
public:
	
	std::vector<ParseNode*> elements;

	virtual ~ArrayLiteralNode() {}


	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		
		children.insert(children.end(), elements.begin(), elements.end());
	}

	virtual std::string printInfo() const {

		std::string result = "array literal";

		return result;
	};
};

class Message : public ParseNodeWithComments {
public:
	std::string name;

	std::vector<ParseNode*> parameters;

	virtual ~Message() {
		
		for (auto p : parameters) {
			delete p;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.insert(children.end(), parameters.begin(), parameters.end());
	}

	virtual std::string printInfo() const {
		std::string result = "message: '" + name + "'";

		return result;
	};
};

class ReturnExpression : public ParseNodeWithComments {
public:
	ParseNode* retVal;

	virtual ~ReturnExpression() {
		delete retVal;
	}

	virtual bool hasChildren() const {
		return true;
	}
	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.push_back(retVal);
	}

	virtual std::string printInfo() const {
		std::string result = "return: ";

		return result;
	};
};

class SendMessage : public ParseNodeWithComments {
public:
	ParseNode* instance;

	Message* message;

	virtual ~SendMessage() {
		delete instance;
		delete message;

	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.push_back(instance);
		children.push_back(message);
	}

	virtual std::string printInfo() const {
		std::string result = "send message:";

		return result;
	};
};


class Assignment : public SendMessage {
public:
	
	virtual ~Assignment() {}


	virtual std::string printInfo() const {
		std::string result = "assignment:";

		return result;
	};
};


class StatementBlock : public ParseNodeWithComments {
public:
	virtual std::string printInfo() const {
		std::string result = "statement:";

		return result;
	};
};

class ClassBlock : public ParseNodeWithComments {
public:
	virtual std::string printInfo() const {
		std::string result = "class:";

		return result;
	};
};




class StatementsBlock : public ParseNodeWithComments {
public:
	std::vector<ParseNode*> statements;

	virtual ~StatementsBlock() {
		for (auto statement : statements) {
			delete statement;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);

		children.insert(children.end(), statements.begin(), statements.end());
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "statements";

		return result;
	};
};


class NamespaceBlock : public ParseNodeWithComments {
public:
	std::string name;
	std::vector<NamespaceBlock*> namespaces;
	StatementsBlock* statements;

	virtual ~NamespaceBlock() {
		for (auto namespaceBlock : namespaces) {
			delete namespaceBlock;
		}

		delete statements;
	}
	

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.insert(children.end(), namespaces.begin(), namespaces.end());
		children.push_back(statements);
	}

	virtual std::string printInfo() const {
		std::string result = "namespace:" + name;

		return result;
	};
};


class CompileFlags : public ParseNode {
public:

	virtual ~CompileFlags() {}


	std::vector<std::string> flags;
	virtual bool hasChildren() const {
		return false;
	}

	virtual std::string printInfo() const {
		std::string result = "flags: ";
		for (auto f: flags) {
			result += f + " ";
		}
		return result;
	};
};

class CodeFragmentBlock : public ParseNodeWithComments {
public:
	std::vector<NamespaceBlock*> namespaces;
	StatementsBlock* statements;

	CompileFlags* flags;

	virtual ~CodeFragmentBlock() {
		for (auto block : namespaces) {
			delete block;
		}

		delete statements;
		delete flags;
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		if (NULL != flags) {
			children.push_back(flags);
		}

		children.insert(children.end(),namespaces.begin(), namespaces.end());
		if (NULL != statements) {
			children.push_back(statements);
		}
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "code fragment";

		return result;
	};
};


class ModuleBlock : public ParseNodeWithComments {
public:
	std::string name;
	virtual ~ModuleBlock() {

		delete codeFragment;

	}

	CodeFragmentBlock* codeFragment = NULL;

	void setCodeFragment(CodeFragmentBlock* v) {
		codeFragment = v;
		codeFragment->parent = this;
	}
	

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		if (codeFragment != NULL) {
			children.push_back(codeFragment);
		}
	}

	virtual std::string printInfo() const {
		std::string result = "module:" + name;

		return result;
	};
};

class AST {
public:
	ParseNode* root = NULL;

	void clear() {
		if (root) {
			delete root;
		}
		root = NULL;

	}

	void printNode(const ParseNode& node) {
		size_t depth = 0;
		const ParseNode* parent = node.parent;
		while (parent != nullptr) {
			depth++;
			parent = parent->parent;
		}

		std::string tabSpacer = "";
		tabSpacer.assign(depth, '\t');

		std::cout << tabSpacer << " ( " << node.printInfo() << std::endl;

		if (node.hasChildren()) {
			std::vector<ParseNode*> children;
			node.getChildren(children);
			for (auto child : children) {
				if (child) {
					printNode(*child);
				}
			}
		}

		std::cout << tabSpacer << " ) " << std::endl;
	}

	void print() {
		
		if (root != nullptr) {
			printNode(*root);
		}
	}
};







class Parser {
public:
		
	class Error {
	public:
		const ParseNode* node = NULL;
		size_t line = 0;
		size_t col = 0;
		size_t offset = 0;
		std::string message;
		std::string errFragment;
		ParseErrorType errCode = UNKNOWN_ERR;

		Error() {}


		void output() const {
			std::string errorText = errFragment;
			errorText += "\n\n";
			std::string spacer;
			if (col > 2) {
				spacer.append(col - 2, '-');
			}
			errorText += spacer + "^" + "\n";
			std::cout << errorText << "Parser error: " << message << " at line : " << line << ", " << col << std::endl;
		}
	};

	enum State {
		NONE = 0,
		COMPILER_FLAGS,
		CODE_FRAGMENT_BLOCK,
		MODULE_BLOCK,
		NAMESPACE_BLOCK,
		STATEMENTS_BLOCK,
		ASSIGNMENT,
		EXPRESSION,
		RETURN_EXPRESSION,
		VARIABLE,
		VARIABLE_DEFINITION,
		SEND_MESSAGE,
		MESSAGE,
		
	};

	std::deque<State> stateStack;
	AST ast;
	

	const FileContext* currentCtx = nullptr;

	Parser(){}

	void clear() {
		ast.clear();
		currentCtx = nullptr;
	}

	void pushState(State s)
	{
		stateStack.push_back(s);
	}

	State popState()
	{
		State result = stateStack.back();
		stateStack.pop_back();
		if (stateStack.empty()) {
			pushState(NONE);
		}
		return result;
	}


	State currentState() const {
		return stateStack.back();
	}

	void clearState() {
		stateStack.clear();
		pushState(NONE);
	}


	void error(const Token& token, const FileContext& ctx, const std::string& errMsg) {
		Parser::Error parseError;
		parseError.node = nullptr;
		parseError.message = errMsg;
		parseError.col = token.colNumber;
		parseError.line = token.lineNumber;
		parseError.offset = token.offset;

		parseError.errFragment = ctx.getLine(parseError.offset);

		throw parseError;
	}

	void error(const Token& token, const FileContext& ctx, const std::string& errMsg, ParseNode* result) {
		delete result;
		popState();
		Parser::Error parseError;
		parseError.node = nullptr;
		parseError.message = errMsg;
		parseError.col = token.colNumber;
		parseError.line = token.lineNumber;
		parseError.offset = token.offset;

		parseError.errFragment = ctx.getLine(parseError.offset);
		throw parseError;
	}

	bool lookAhead(Token::Type* types, size_t typesSize, const FileContext& ctx)
	{
		std::vector<Token> tokens;
		prevToken(ctx);

		bool res = peekNext(ctx, typesSize, tokens);

		nextToken(ctx, NULL, false);

		if (!res) {
			return false;
		}

		size_t i = 0;
		for (auto t : tokens) {
			if (t.type != types[i]) {
				return false;
			}
			++i;
		}
		return true;
	}

	void verifyTokenTypeOrFail(const Token& t, Token::Type type, const FileContext& ctx, const std::string& errMsg, ParseNode* node)
	{
		if (t.type != type) {
			error(t, ctx, errMsg, node);
		}
	}

	void verifyTokenTypeOrFail(const Token& t, Token::Type* types, size_t typesSize, const FileContext& ctx, const std::string& errMsg, ParseNode* node)
	{
		int errCount = 0;
		for (size_t i = 0;i < typesSize;++i) {
			auto type = types[i];
			if (t.type != type) {
				errCount++;
			}
		}

		if (errCount >= typesSize) {
			error(t, ctx, errMsg, node);
		}
	}

	void verifyTokenOrFail(const Token& t, Token::Type type, const std::string& expectedValue, const FileContext& ctx, 
		const std::string& errMsg, ParseNode* node)
	{		
		if (t.type != type || t.text != expectedValue) {
			error(t, ctx, errMsg, node);
		}
	}

	bool beginToken(const FileContext& ctx, ParseNode* parent) {
		ctx.beginTokens();

		if (ctx.tokenCount() == 0) {
			return false;
		}

		this->commentCheck(ctx, parent);

		return true;
	}

	Token peekNext(const FileContext& ctx) {
		Token result;

		ctx.peekNextToken(result);

		return result;
	}


	bool peekNext(const FileContext& ctx, size_t tokenCount, std::vector<Token>& tokens) {
				
		return ctx.peekNextTokens(tokenCount, tokens);
	}


	Token peekPrev(const FileContext& ctx) {
		Token result;

		ctx.peekPrevToken(result);

		return result;
	}

	bool nextToken(const FileContext& ctx, ParseNode* parent, bool checkForComments=true) {
		
		if (!ctx.nextToken()) {
			return false;
		}

		if (checkForComments) {
			this->commentCheck(ctx, parent);
		}

		return true;
	}

	bool prevToken(const FileContext& ctx) {
		return ctx.prevToken();
	}

	ClassBlock* classBlock(const FileContext& ctx, ParseNode* parent) {
		ClassBlock* result = nullptr;

		return result;
	}


	class ParseStateGuard {
	public:
		size_t stateDepth = 0;
		Parser& parser;
		Parser::State state;
		const FileContext& ctx;
		std::string errMsg;
		ParseNode** resultPtr=NULL;
		ParseStateGuard(Parser& p, Parser::State s, const FileContext& c, const std::string& msg, ParseNode* r):
			parser(p), 
			state(s), 
			ctx(c), 
			errMsg(msg) ,
			resultPtr(&r)
		{
			parser.pushState(state);
			stateDepth = parser.stateStack.size();
		}

		~ParseStateGuard() {
			if (stateDepth != parser.stateStack.size() || parser.currentState() != state) {
				ParseNode* n = NULL;
				if (resultPtr != NULL) {
					n = *resultPtr;
				}

				parser.error(ctx.getCurrentToken(), ctx, errMsg, n);
			}

			parser.popState();
		}
	};

	ParseNode* returnExpression(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;

		ParseStateGuard pg(*this, RETURN_EXPRESSION, ctx, "Parsing return statement, invalid state exiting parse", result);

		result = new ReturnExpression();
		result->parent = parent;


		return result;
	}

	VariableNode* variableDef(const FileContext& ctx, ParseNode* parent) {
		VariableNode* result = nullptr;

		ParseStateGuard pg(*this, VARIABLE_DEFINITION, ctx, "Parsing variable definition, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::IDENTIFIER,
			ctx,
			"Parsing variable, expected identifier",
			result);

		auto tok = ctx.getCurrentToken();

		result = new VariableNode();
		result->parent = parent;

		result->name = tok.text.str();

		nextToken(ctx, parent);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::COLON,
			ctx,
			"Parsing variable definition, expected colon",
			result);
		

		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			Token::IDENTIFIER,
			ctx,
			"Parsing variable, expected identifier for type",
			result);


		result->type = tok.text.str();

		


		return result;
	}


	VariableNode* variable(const FileContext& ctx, ParseNode* parent) {
		VariableNode* result = nullptr;

		ParseStateGuard pg(*this, VARIABLE, ctx, "Parsing variable, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::IDENTIFIER,
			ctx,
			"Parsing variable, expected identifier",
			result);

		auto tok = ctx.getCurrentToken();

		Token::Type types[] = { Token::IDENTIFIER, Token::COLON, Token::IDENTIFIER};
		
		if (lookAhead(types, sizeof(types) / sizeof(types[0]), ctx)) {
			result = variableDef(ctx, parent);
		}
		else {
			result = new VariableNode();
			result->parent = parent;

			result->name = tok.text.str();

		}




		


		return result;
	}

	ParseNode* expression(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;

		ParseStateGuard pg(*this, EXPRESSION, ctx, "Parsing expression, invalid state exiting parse", result);


		auto first = ctx.getCurrentToken();
		if (first.type == Token::KEYWORD) {
			if (first.text == "return") {
				result = returnExpression(ctx, parent);
			}
		}
		else if (first.type == Token::OPEN_PAREN) {
			//should be expression!
			nextToken(ctx, parent);
			result = expression(ctx, parent);

			nextToken(ctx, parent);

			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				Token::CLOSE_PAREN,
				ctx,
				"Parsing grouped expression, expected closing paren ')'",
				result);
		}
		else if (first.type == Token::IDENTIFIER) {
			//send message
			
			result = sendMessage(ctx, parent);
		}
		else {
			error(ctx.getCurrentToken(), ctx, "Parsing expression, invalid syntax?", result);
		}
		
		return result;
	}

	ParseNode* sendMessage(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;

		ParseStateGuard pg(*this, SEND_MESSAGE, ctx, "Parsing send message, invalid state exiting parse", result);

		SendMessage* sendMsg = new SendMessage();
		sendMsg->parent = parent;
		result = sendMsg;

		auto tok = ctx.getCurrentToken();

		InstanceNode* instance = new InstanceNode();
		instance->parent = sendMsg;
		instance->name = tok.text.str();

		sendMsg->instance = instance;

		nextToken(ctx, sendMsg);

		auto msg = message(ctx, sendMsg);
		sendMsg->message = msg;
		
		result = sendMsg;

		return result;
	}

	ParseNode* assignment(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;
		
		Assignment* assignmentNode = new Assignment();
		assignmentNode->parent = parent;

		ParseStateGuard pg(*this, ASSIGNMENT, ctx, "Parsing assignment, invalid state exiting parse", result);

		result = assignmentNode;



		auto tokIdent = ctx.getCurrentToken();

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::IDENTIFIER,
			ctx,
			"Parsing assignment, expected identier for variable/instance name",
			result);

		nextToken(ctx, assignmentNode);

		auto tok = ctx.getCurrentToken();

		if (tok.type == Token::COLON) {
			prevToken(ctx);
			VariableNode* var = variable(ctx, assignmentNode);
			var->parent = assignmentNode;

			assignmentNode->instance = var;

		}
		else {
			prevToken(ctx);
			//expression?
			InstanceNode* instance = new InstanceNode();
			instance->parent = assignmentNode;
			instance->name = tokIdent.text.str();
			assignmentNode->instance = instance;
		}

		nextToken(ctx, assignmentNode);

		auto assignmentOpMsg = message(ctx, assignmentNode);

		assignmentNode->message = assignmentOpMsg;

		return result;
	}


	ParseNode* varLiteral(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = NULL;

		auto tok = ctx.getCurrentToken();
		switch (tok.type) {
			case Token::OPEN_BRACKET: {
				bool arrayClosed = false;
				ArrayLiteralNode* arrayLit = new ArrayLiteralNode();
				arrayLit->parent = parent;

				while (nextToken(ctx,parent) && !arrayClosed) {
					tok = ctx.getCurrentToken();

					if (tok.isLiteral()) {
						ParseNode* lit = varLiteral(ctx, arrayLit);
						arrayLit->elements.push_back(lit);
					}
					else {
						switch (tok.type) {
							case Token::CLOSE_BRACKET: {
								arrayClosed = true;
								break; //loop
							}
							break;

							case Token::END_OF_STATEMENT: {
									
								break; //loop
							}
							break;

							case Token::IDENTIFIER: {
								InstanceNode* instance = new InstanceNode();
								instance->parent = arrayLit;
								instance->name = tok.text.str();
								arrayLit->elements.push_back(instance);
							}
							break;

							case Token::COMMA: {
								
							}
							break;

							default: {
								error(ctx.getCurrentToken(), ctx, "Parsing array literal, invalid tokens found", arrayLit);
							}
							break;
						}
					}
				}
				if (!arrayClosed) {
					//error
					error(ctx.getCurrentToken(), ctx, "Parsing array literal, no array close found", arrayLit);
				}

				if (ctx.getCurrentToken().type == Token::END_OF_STATEMENT) {
					prevToken(ctx);
				}

				result = arrayLit;
			} break;

			case Token::INTEGER_LITERAL: case Token::BOOLEAN_LITERAL:
			case Token::BINARY_LITERAL: case Token::DECIMAL_LITERAL:
			case Token::HEXADECIMAL_LITERAL: case Token::STRING_LITERAL: {
				LiteralNode* literal = new LiteralNode();
				literal->parent = parent;
				literal->val = tok.text.str();
				switch (tok.type) {
				case Token::INTEGER_LITERAL:literal->type = LiteralNode::INTEGER_LITERAL;break;
				case Token::BOOLEAN_LITERAL:literal->type = LiteralNode::BOOLEAN_LITERAL;break;
				case Token::BINARY_LITERAL:literal->type = LiteralNode::BINARY_LITERAL;break;
				case Token::HEXADECIMAL_LITERAL:literal->type = LiteralNode::HEXADECIMAL_LITERAL;break;
				case Token::DECIMAL_LITERAL:literal->type = LiteralNode::DECIMAL_LITERAL;break;
				case Token::STRING_LITERAL:literal->type = LiteralNode::STRING_LITERAL;break;
				}
				result = literal;

			} break;
		}
		return result;
	}


	Message* message(const FileContext& ctx, ParseNode* parent) {
		Message* result = NULL;

		
		pushState(MESSAGE);
		auto stateDepth = stateStack.size();

		Token::Type types []  = {Token::IDENTIFIER,Token::ASSIGMENT_OPERATOR };

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			types,sizeof(types)/sizeof(Token::Type),
			ctx,
			"Parsing message, expected identier",
			result);

		auto msgName = ctx.getCurrentToken();
		

		result = new Message();
		result->parent = parent;
		result->name = msgName.text.str();

		nextToken(ctx, result);
		auto tok = ctx.getCurrentToken();

		if (msgName.type == Token::IDENTIFIER) {
			//name message, look for params
			if (tok.type == Token::COLON) {
				//one or more params...
				nextToken(ctx, result);
				tok = ctx.getCurrentToken();

				if (tok.type == Token::END_OF_STATEMENT) {
					error(ctx.getCurrentToken(), ctx, "Parsing message, got end of statment, expecting params", result);
				}
			}
			else if (tok.type == Token::END_OF_STATEMENT) {
				//back up one, will be picked from previous 
				prevToken(ctx);
			}
		}
		else if (msgName.type == Token::ASSIGMENT_OPERATOR) {
			switch (tok.type) {
				case Token::IDENTIFIER: {

				} break;

				case Token::OPEN_BRACKET: case Token::INTEGER_LITERAL: 
				case Token::BOOLEAN_LITERAL:
				case Token::BINARY_LITERAL: case Token::DECIMAL_LITERAL:
				case Token::HEXADECIMAL_LITERAL : case Token::STRING_LITERAL : {
					ParseNode* literal = varLiteral(ctx, result);
					result->parameters.push_back(literal);

				} break;

				default: {
					error(ctx.getCurrentToken(), ctx, "Parsing message, expecting single param, RHS of assignment operator", result);
				}
			}
		}
		else {
			//wtf???
			error(ctx.getCurrentToken(), ctx, "Parsing message, invalid token", result);
		}


		if (stateDepth != stateStack.size() || currentState() != MESSAGE) {
			error(ctx.getCurrentToken(), ctx, "Parsing message, invalid state exiting parse", result);
		}

		popState();

		return result;
	}

	ParseNode* statement(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;

		auto first = ctx.getCurrentToken();
		
		if (first.type == Token::IDENTIFIER) {
			nextToken(ctx, parent);
			auto tok = ctx.getCurrentToken();

			if (tok.type == Token::ASSIGMENT_OPERATOR) {
				prevToken(ctx);
				result = assignment(ctx, parent);
			}
			else if (tok.type == Token::COLON) {
				//could be a var definition (i.e. foo:int8) 
				//or could be assignment ( foo:int8 := 99)
				//check for tyerminating ';' char
				Token::Type types[] = {Token::COLON, Token::IDENTIFIER, Token::END_OF_STATEMENT};
				if (lookAhead(types, sizeof(types)/sizeof(types[0]), ctx)) {
					prevToken(ctx);
					result = variableDef(ctx, parent);
				}
				else {
					prevToken(ctx);

					result = assignment(ctx, parent);
				}
			}
			else {
				prevToken(ctx);
				result = expression(ctx, parent);
			}

			nextToken(ctx,result);
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				Token::END_OF_STATEMENT,
				ctx,
				"Parsing statement, expected end of statement: ';'",
				result);
		}
		else if (first.type == Token::OPEN_PAREN) {
			//expression
			result = expression(ctx, parent);
		}
		else {
			error(ctx.getCurrentToken(), ctx, "Parsing statement, invalid code", result);
		}

		return result;
	}

	ParseNode* statementOrCommentOrClass(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;
		auto first = ctx.getCurrentToken();

		if (first.type == Token::KEYWORD && first.text == "class") {
			result = classBlock(ctx, parent);
		}
		else {
			result = statement(ctx, parent);
		}
		

		return result;
	}

	NamespaceBlock* namespaceBlock(const FileContext& ctx, ParseNode* parent) {
		NamespaceBlock* result = nullptr;

		pushState(NAMESPACE_BLOCK);
		size_t stateDepth = stateStack.size();
						
		verifyTokenOrFail(ctx.getCurrentToken(), 
			Token::KEYWORD, 
			"namespace", 
			ctx, 
			"Parsing namespace, expected namespace keyword", 
			result);

		nextToken(ctx,parent);

		auto& namespaceIdToken = ctx.getCurrentToken();		

		nextToken(ctx, parent);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::OPEN_BLOCK,
			ctx,
			"Parsing namespace, expected open brace '{'",
			result);


		nextToken(ctx, parent);
		auto& nextTok = ctx.getCurrentToken();

		//ok at this point we potentially have a namespace. go ahead an allocate 
		//an object

		result = new NamespaceBlock();
		result->parent = parent;
		result->name = namespaceIdToken.text.str();		

		

		//expecting a namespace or statements
		
		bool checkForClose = true;
		if (nextTok.type == Token::KEYWORD && nextTok.text == "namespace") {
			NamespaceBlock* childNamespace = namespaceBlock(ctx, result);
			result->namespaces.push_back(childNamespace);
		}
		else if (nextTok.type == Token::CLOSE_BLOCK ) {
			checkForClose = false;
		}
		else { //expecting statements
			result->statements = statements(ctx, result);
		}
		
		if (checkForClose) {
			nextToken(ctx, result);
		}	

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::CLOSE_BLOCK,
			ctx,
			"Parsing namespace, expected closing brace '}'",
			result);

		if (stateDepth != stateStack.size() || currentState() != NAMESPACE_BLOCK) {
			error(ctx.getCurrentToken(), ctx, "Parsing namespace, invalid state exiting parse", result);
		}

		popState();

		return result;
	}

	StatementsBlock* statements(const FileContext& ctx, ParseNode* parent)
	{
		StatementsBlock* result = new StatementsBlock();
		result->parent = parent;
		
		pushState(STATEMENTS_BLOCK);

		auto statementBlock = statementOrCommentOrClass(*currentCtx, result);
		while (statementBlock != NULL) {
			result->statements.push_back(statementBlock);

			nextToken(*currentCtx, statementBlock);

			auto& tok = currentCtx->getCurrentToken();
			if (tok.type == Token::CLOSE_BLOCK) {
				break;
			}

			statementBlock = statementOrCommentOrClass(*currentCtx, result);
		}

		popState();

		return result;
	}

	CompileFlags* compilerFlags(const FileContext& ctx, ParseNode* parent) {
		CompileFlags* flags = new CompileFlags();

		ParseStateGuard pg(*this, COMPILER_FLAGS, ctx, "Parsing compiler flags, invalid state exiting parse", flags);

		flags->parent = parent;
		auto tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			Token::AT_SIGN,
			ctx,
			"Expected '@' to start flags",
			NULL);

		nextToken(ctx, parent);
		tok = ctx.getCurrentToken();
		if (tok.type == Token::OPEN_BLOCK) {
			return flags;
		}
		else {
			verifyTokenTypeOrFail(tok,
				Token::OPEN_BRACKET,
				ctx,
				"Expected '[' to start flags",
				NULL);

			
			std::string curFlag;
			while (nextToken(ctx, parent)) {
				
				tok = ctx.getCurrentToken();
				if (tok.type == Token::IDENTIFIER) {
					curFlag = tok.text.str();
				}
				else if (tok.type == Token::COMMA) {
					flags->flags.push_back(curFlag);
					curFlag = "";
				}
				else if (tok.type == Token::CLOSE_BRACKET) {
					if (!curFlag.empty()) {
						flags->flags.push_back(curFlag);
					}
					
					break;
				}
				else {
					error(ctx.getCurrentToken(), ctx, "Parsing compiler flags, invalid syntax", flags);
				}
			}
			
			

		}

		return flags;
	}

	CodeFragmentBlock* codeFragmentBlock(const FileContext& ctx, ParseNode* parent) {
		CodeFragmentBlock* result = nullptr;

		Token::Type types[] = { Token::OPEN_BLOCK, Token::AT_SIGN };

		auto tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			types,sizeof(types)/sizeof(types[0]),
			ctx,
			"Code Fragment Expected open block '{' or '@' for flags",
			result);

		verifyTokenTypeOrFail(ctx.lastToken(),
			Token::CLOSE_BLOCK,
			ctx,
			"Code Fragment Expected close block '}'",
			result);
		

		CodeFragmentBlock* block = new CodeFragmentBlock();
		result = block;
		pushState(CODE_FRAGMENT_BLOCK);
		auto stateDepth = stateStack.size();

		if (tok.type == Token::AT_SIGN) {
			//process flags
			CompileFlags* flags = compilerFlags(ctx, block);
			block->flags = flags;
			nextToken(ctx, block);

			tok = ctx.getCurrentToken();
			verifyTokenTypeOrFail(tok,
				Token::OPEN_BLOCK,
				ctx,
				"Code Fragment Expected open block '{' or '@' for flags",
				result);
		}
		

		


		nextToken(ctx, block);
		
		tok = ctx.getCurrentToken();
		
		if (tok.type == Token::KEYWORD && tok.text == "namespace") {
			auto namespaceBlk = namespaceBlock(ctx, block);
			while (namespaceBlk != NULL) {
				block->namespaces.push_back(namespaceBlk);

				nextToken(ctx, namespaceBlk);

				auto& tok = ctx.getCurrentToken();
				if (tok.type == Token::CLOSE_BLOCK) {
					break;
				}

				namespaceBlk = namespaceBlock(ctx, block);
			}
		}
		else {
			StatementsBlock* res =  statements(ctx, block);
			if (res) {
				block->statements = res;
			}
		}
		

		if (stateDepth != stateStack.size() || currentState() != CODE_FRAGMENT_BLOCK) {
			error(ctx.getCurrentToken(), ctx, "Parsing code fragment, invalid state exiting parse", result);
			return nullptr;
		}

		popState();

		return result;
	}

	ParseNode* moduleBlock(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;


		pushState(MODULE_BLOCK);
		size_t stateDepth = stateStack.size();

		verifyTokenOrFail(ctx.getCurrentToken(),
			Token::KEYWORD,
			"module",
			ctx,
			"Parsing module, expected module keyword",
			result);

		nextToken(ctx, parent);

		auto& moduleIdToken = ctx.getCurrentToken();

		nextToken(ctx, parent);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::OPEN_BLOCK,
			ctx,
			"Parsing module, expected open brace '{'",
			result);

		verifyTokenTypeOrFail(ctx.lastToken(),
			Token::CLOSE_BLOCK,
			ctx,
			"module expected close block '}'",
			result);



		ModuleBlock* module = new ModuleBlock();
		module->name = moduleIdToken.text.str();
		result = module;

		CodeFragmentBlock* codeFrag = codeFragmentBlock(ctx, module);
		if (NULL == codeFrag) {
			error(ctx.getCurrentToken(), ctx, "Parsing module, missing or invalid code fragment", result);
			return nullptr;
		}

		module->setCodeFragment(codeFrag);


		if (stateDepth != stateStack.size() || currentState() != MODULE_BLOCK) {
			error(ctx.getCurrentToken(), ctx, "Parsing module, invalid state exiting parse", result);
			return nullptr;
		}

		popState();

		return result;
	}

	ParseNode* programBlock() {
		ParseNode* result = nullptr;

		return result;
	}

	ParseNode* libraryBlock() {
		ParseNode* result = nullptr;

		return result;
	}

	void comment(const FileContext& ctx, ParseNode* parent) {
		auto& tok = ctx.getCurrentToken();

		Token::Type types[] = { Token::COMMENT,Token::COMMENT_START };
		verifyTokenTypeOrFail(tok,
			types, sizeof(types)/sizeof(types[0]),
			ctx,
			"Parsing comment, expected module keyword",
			NULL);


		
		
		Comment* commentNode = NULL;

		if (tok.type == Token::COMMENT_START) {
			if (!nextToken(ctx, parent,false)) {
				error(tok, ctx, "handling multi line comments, expected comment body");
			}
			auto& comment = ctx.getCurrentToken();

			verifyTokenTypeOrFail(comment,
				Token::COMMENT,
				ctx,
				"Parsing comment, expected module keyword",
				NULL);

			if (!nextToken(ctx,parent,false)) {
				error(tok, ctx, "handling multi line comments, expected comment close");
			}
			auto& commentEnd = ctx.getCurrentToken();

			verifyTokenTypeOrFail(commentEnd,
				Token::COMMENT_END,
				ctx,
				"Parsing comment, expected module keyword",
				NULL);

			commentNode = new Comment();
			commentNode->parent = parent;
			commentNode->comments = comment.text.str();
		}
		else {
			commentNode = new Comment();
			commentNode->parent = parent;
			commentNode->comments = tok.text.str();
		}
		
		if (NULL != commentNode) {
			ParseNodeWithComments* pnc = dynamic_cast<ParseNodeWithComments*>(parent);
			if (NULL != pnc) {
				pnc->comments.push_back(commentNode);
			}
		}
	}

	void commentCheck(const FileContext& ctx, ParseNode* parent) {
		const Token& token = ctx.getCurrentToken();

		if (token.type == Token::COMMENT_START || token.type == Token::COMMENT) {
			comment(*currentCtx, parent);
			nextToken(ctx, NULL,false);
		}
	}


	bool start() {
		bool result = true;
		currentCtx->beginTokens();

		beginToken(*currentCtx,NULL);

		do {
			

			const Token& token = currentCtx->getCurrentToken();

			switch (token.type) {
				case Token::AT_SIGN: {
					auto tok = peekNext(*currentCtx);
					if (tok.type == Token::OPEN_BLOCK || tok.type == Token::OPEN_BRACKET) {
						ast.root = codeFragmentBlock(*currentCtx, NULL);
					}
					result = ast.root != nullptr;
				}
				break;

				case Token::OPEN_BLOCK: {
					ast.root = codeFragmentBlock(*currentCtx, NULL);
					result = ast.root != nullptr;
				}
				break;

				case Token::KEYWORD: {
					if (token.text == "module") {
						ast.root = moduleBlock(*currentCtx, NULL);
					}
					else if (token.text == "program") {
						ast.root = programBlock();
					}
					else if (token.text == "lib") {
						ast.root = libraryBlock();
					}
					else {
						error(token, *currentCtx, "Invalid keyword, expected one of module, lib, or program");
						return false;
					}
				}
				break;

				default: {
					error(token, *currentCtx, "Unknown error");
					return false;
				}
				break;
			}
		} while (nextToken(*currentCtx, ast.root));

		return result;
	}

	bool parse(const Lexer& lexer) {
		clear();
		currentCtx = lexer.currentContext();
		ast.root = nullptr;	

		return start();
	}
};


#endif //_PARSER_H__



