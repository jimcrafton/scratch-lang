#ifndef _PARSER_H__
#define _PARSER_H__

#include <functional>

#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <map>


#include "Lexer.h"
#include "AST.h"

namespace utils  {
	class cmd_line_options;
}

namespace language  {
	class AstVisitor;
}

namespace parser {

class ParserOptions {
public:
	bool verboseMode = false;
	bool debugMode = false;
	
	void init(const utils::cmd_line_options& cmdline);
};





class TupleNode : public language::ParseNode {
public:
	std::vector<std::string> fields;

	virtual ~TupleNode() {
	}

	virtual bool hasChildren() const {
		return language::ParseNode::hasChildren();
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
	}

	virtual std::string printInfo() const {
		std::string result = "tuple: " + name + " [";
		std::string tmp;
		for (auto t : fields) {
			if (!tmp.empty()) {
				tmp += ", ";
			}
			tmp += t;
		}
		if (!tmp.empty()) {
			result += tmp;
		}

		result += "]";
		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class NamedTupleNode : public language::ParseNode {
public:
	typedef std::pair < std::string, std::string> TupleFieldT; //first: name of field, second: type
	std::vector<TupleFieldT> fields;

	virtual ~NamedTupleNode() {
	}

	virtual bool hasChildren() const {
		return language::ParseNode::hasChildren();
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
	}

	virtual std::string printInfo() const {
		std::string result = "named tuple: " + name + " [";
		std::string tmp;
		for (auto t : fields) {
			if (!tmp.empty()) {
				tmp += ", ";
			}
			tmp += t.first + " : " + t.second;
		}
		if (!tmp.empty()) {
			result += tmp;
		}

		result += "]";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class ParamNode : public language::ParseNode {
public:
	std::string type;

	virtual ~ParamNode() {
	}

	virtual std::string printInfo() const {
		std::string result = "parameter: " + name + " " + type;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class VariableNode : public language::ParseNode {
public:
	
	std::string type;

	virtual ~VariableNode() {
	}



	virtual std::string printInfo() const {
		std::string result = "var: " + name + " " + type;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class InstanceNode : public language::ParseNode {
public:
	
	std::string type;

	virtual ~InstanceNode() {
	}



	virtual std::string printInfo() const {
		std::string result = "instance: " + name;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};





class NilNode : public language::ParseNode {
public:
	std::string type;

	virtual ~NilNode() {
	}



	virtual std::string printInfo() const {
		std::string result = "nil ";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};




class LiteralNode : public language::ParseNode {
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

	virtual void accept(language::AstVisitor& v) const;
};

class ArrayLiteralNode : public language::ParseNode {
public:
	
	std::vector<language::ParseNode*> elements;

	virtual ~ArrayLiteralNode() {}


	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		
		children.insert(children.end(), elements.begin(), elements.end());
	}

	virtual std::string printInfo() const {

		std::string result = "array literal";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class GroupedExpression : public language::ParseNode {
public:

	language::ParseNode* innerExpression = nullptr;

	virtual ~GroupedExpression() {
		delete innerExpression;
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		if (nullptr != innerExpression) {
			children.insert(children.end(), innerExpression);
		}
		
	}

	virtual std::string printInfo() const {
		std::string result = "group expression";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};


class MessageParam : public language::ParseNode {
public:
	ParseNode* param = nullptr;	

	virtual ~MessageParam() {
		delete param;		
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);

		if (nullptr != param) {
			children.push_back(param);
		}
	}

	virtual std::string printInfo() const {
		std::string result = "param: '" + name + "'";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class Message : public language::ParseNode {
public:
	
	language::ParseNode* returnVal = nullptr;

	std::vector<MessageParam*> parameters;

	virtual ~Message() {
		
		for (auto p : parameters) {
			delete p;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		children.insert(children.end(), parameters.begin(), parameters.end());

		if (returnVal) {
			children.push_back(returnVal);
		}
	}

	virtual std::string printInfo() const {
		std::string result = "message: '" + name + "'";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class ReturnExpression : public language::ParseNode {
public:
	language::ParseNode* retVal;

	virtual ~ReturnExpression() {
		delete retVal;
	}

	virtual bool hasChildren() const {
		return true;
	}
	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		children.push_back(retVal);
	}

	virtual std::string printInfo() const {
		std::string result = "return: ";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class SendMessage : public language::ParseNode {
public:
	language::ParseNode* instance;

	Message* message;

	virtual ~SendMessage() {
		delete instance;
		delete message;
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		children.push_back(instance);
		children.push_back(message);
	}

	virtual std::string printInfo() const {
		std::string result = "send message:";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};


class Assignment : public SendMessage {
public:
	
	virtual ~Assignment() {}


	virtual std::string printInfo() const {
		std::string result = "assignment:";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};


class StatementBlock : public language::ParseNode {
public:
	virtual std::string printInfo() const {
		std::string result = "statement:";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class ScopeNode : public language::ParseNode {
public:
	std::string scope;

	std::vector<VariableNode*> memberVars;

	virtual ~ScopeNode() {
		for (auto mv : memberVars) {
			delete mv;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);

		children.insert(children.end(), memberVars.begin(), memberVars.end());
	}

	virtual std::string printInfo() const {
		std::string result = "scope: " + scope;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};



class StatementsBlock : public language::ParseNode {
public:
	std::vector<language::ParseNode*> statements;

	virtual ~StatementsBlock() {
		for (auto statement : statements) {
			delete statement;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);

		children.insert(children.end(), statements.begin(), statements.end());
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "statements";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};


class MessageDeclaration : public language::ParseNode {
public:
	std::vector<ParamNode*> params;
	bool msgIsClosure = false;

	TupleNode* returnType = nullptr;

	virtual ~MessageDeclaration() {
		for (auto p : params) {
			delete p;
		}

		delete returnType;
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		children.insert(children.end(), params.begin(), params.end());

		if (nullptr != returnType) {
			children.push_back(returnType);
		}

	}

	virtual std::string printInfo() const {
		std::string result = "message declaration: '" + name + "' (closure: "; 
		result += msgIsClosure ? "true" : "false";
		result += ")";
		if (nullptr == returnType) {
			result += ", no return type defined";
		}

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};

class MessageBlock : public language::ParseNode {
public:
	MessageDeclaration* msgDecl = nullptr;
	StatementsBlock* statements;

	virtual ~MessageBlock() {
		delete msgDecl;
		delete statements;
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		if (nullptr != msgDecl) {
			children.push_back(msgDecl);
		}

		if (nullptr != statements) {
			children.push_back(statements);
		}
	}

	virtual std::string printInfo() const {
		std::string result = "message block: '" + name + "'";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;

};



class MsgInstanceNode : public language::ParseNode {
public:

	//MessageDeclaration* msgDecl = nullptr;
	MessageBlock* msgBlock = nullptr;

	virtual ~MsgInstanceNode() {
		//delete msgDecl;
		delete msgBlock;
	}

	virtual bool hasChildren() const {
		return language::ParseNode::hasChildren() || (/*nullptr != msgDecl || */nullptr != msgBlock);
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);

		//if (nullptr != msgDecl) {
			//children.push_back(msgDecl);
		//}

		if (nullptr != msgBlock) {
			children.push_back(msgBlock);
		}
	}


	virtual std::string printInfo() const {
		std::string result = "msg instance: ";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};



class ClassBlock : public language::ParseNode {
public:
	
	std::string super;

	std::vector<ScopeNode*> scopes;
	std::vector<MessageBlock*> messages;
	

	virtual ~ClassBlock() {
		for (auto mv : scopes) {
			delete mv;
		}

		for (auto m : messages) {
			delete m;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);

		children.insert(children.end(), scopes.begin(), scopes.end());

		children.insert(children.end(), messages.begin(), messages.end());
	}

	virtual std::string printInfo() const {
		std::string result = "class: " + name + ", super: " + super;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};



class RecordBlock : public language::ParseNode {
public:
	
	std::string super;
	std::vector<VariableNode*> memberVars;

	virtual ~RecordBlock() {
		for (auto mv : memberVars) {
			delete mv;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);

		children.insert(children.end(), memberVars.begin(), memberVars.end());
	}

	virtual std::string printInfo() const {
		std::string result = "record: " + name + ", super: " + super;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};



class NamespaceBlock : public language::ParseNode {
public:
	
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

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		children.insert(children.end(), namespaces.begin(), namespaces.end());
		children.push_back(statements);
	}

	virtual std::string printInfo() const {
		std::string result = "namespace:" + name;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};


class CodeFragmentBlock : public language::ParseNode {
public:
	std::vector<NamespaceBlock*> namespaces;
	StatementsBlock* statements;

	
	virtual ~CodeFragmentBlock() {
		for (auto block : namespaces) {
			delete block;
		}

		delete statements;		
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
	

		children.insert(children.end(),namespaces.begin(), namespaces.end());
		if (nullptr != statements) {
			children.push_back(statements);
		}
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "code fragment";

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};


class ModuleBlock : public language::ParseNode {
public:
	
	std::string version;
	virtual ~ModuleBlock() {

		delete codeFragment;

	}

	CodeFragmentBlock* codeFragment = nullptr;

	void setCodeFragment(CodeFragmentBlock* v) {
		codeFragment = v;
		codeFragment->parent = this;
	}
	

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		language::ParseNode::getChildren(children);
		if (codeFragment != nullptr) {
			children.push_back(codeFragment);
		}
	}

	virtual std::string printInfo() const {
		std::string result = "module:" + name;

		return result;
	};

	virtual void accept(language::AstVisitor& v) const;
};




	enum ParseErrorType {
		UNKNOWN_ERR = 0,
	};

	enum State {
		NONE = 0,
		COMPILER_FLAGS,
		CODE_FRAGMENT_BLOCK,
		MODULE_BLOCK,
		NAMESPACE_BLOCK,
		STATEMENTS_BLOCK,
		STATEMENT,
		CLASS,
		RECORD,
		ASSIGNMENT,
		EXPRESSION,
		GROUPED_EXPRESSION,
		RETURN_EXPRESSION,
		RETURN_DECLARATION,
		TYPE_DECLARATION,
		PARAM_DECLARATION,
		VARIABLE,
		VARIABLE_DEFINITION,
		INSTANCE_EXPR,
		INSTANCE,
		MSG_INSTANCE,
		SEND_MESSAGE,
		MESSAGE,
		MESSAGE_DECL,
		MESSAGE_BLOCK,
		MESSAGE_PARAM,
		ERROR,
	};

	const std::vector<std::string> StateStrings = {
		"NONE",
		"COMPILER_FLAGS",
		"CODE_FRAGMENT_BLOCK",
		"MODULE_BLOCK",
		"NAMESPACE_BLOCK",
		"STATEMENTS_BLOCK",
		"CLASS",
		"RECORD",
		"ASSIGNMENT",
		"EXPRESSION",
		"GROUPED_EXPRESSION",
		"RETURN_EXPRESSION",
		"RETURN_DECLARATION",
		"TYPE_DECLARATION",
		"PARAM_DECLARATION",
		"VARIABLE",
		"VARIABLE_DEFINITION",
		"INSTANCE",
		"INSTANCE_EXPR",
		"SEND_MESSAGE",
		"MESSAGE",
		"MESSAGE_DECL",
		"MESSAGE_BLOCK",
		"ERROR"
	};

class Parser {
public:
		
	

	class Error {
	public:
		const language::ParseNode* node = nullptr;
		
		lexer::Token token;

		std::string message;
		std::string errFragment;
		ParseErrorType errCode = UNKNOWN_ERR;
		std::deque<State> stateStack;

		Error() {}

		Error(const lexer::Token& token, const lexer::FileContext& ctx, const std::string& errMsg, const std::deque<State>& state) {
			this->node = nullptr;
			this->message = errMsg;
			this->token = token;

			this->errFragment = ctx.getLine(token.location.offset);

			if (!state.empty()) {
				this->stateStack = state;
			}
		}

		Error(const lexer::Token& token, const lexer::FileContext& ctx, const std::string& errMsg) {
			this->node = nullptr;
			this->message = errMsg;
			this->token = token;
			this->errFragment = ctx.getLine(token.location.offset);
		}

		void output() const {
			std::string errorText = errFragment;
			errorText += "\n\n";
			std::string spacer;
			if (token.location.colNumber > 2) {
				spacer.append(token.location.colNumber - 2, '-');
			}
			errorText += spacer + "^" + "\n";
			std::cout << errorText << "Parser error: " << message << " at line : " << token.location.lineNumber << ", " << token.location.colNumber << std::endl;
			std::cout << "current token: " << token.info() << std::endl;
			if (!stateStack.empty()) {
				
				std::cout << "State stack:" << std::endl;
				int i = 0;
				for (auto s : stateStack) {
					if (s != parser::ERROR) {
						std::string tabs;
						tabs.append(i * 2, ' ');
						std::cout << tabs << StateStrings[s] << std::endl;
						i++;
					}
				}
			}
		}
	};

	

	class ParseStateGuard {
	public:
		size_t stateDepth = 0;
		Parser& parser;
		parser::State state;
		const lexer::FileContext& ctx;
		std::string errMsg;
		const language::ParseNode** resultPtr = nullptr;
		ParseStateGuard(Parser& p, parser::State s, const lexer::FileContext& c, const std::string& msg, const language::ParseNode* r) :
			parser(p),
			state(s),
			ctx(c),
			errMsg(msg),
			resultPtr(nullptr)
		{
			parser.pushState(state);
			stateDepth = parser.stateStack.size();
			if (nullptr != r) {
				resultPtr = &r;
			}
		}

		~ParseStateGuard() {
			if (stateDepth != parser.stateStack.size() || parser.currentState() != state) {
				const language::ParseNode* n = nullptr;
				if (resultPtr != nullptr) {
					n = *resultPtr;
				}

				if (parser.currentState() != parser::ERROR) {
					parser.error(ctx.getCurrentToken(), ctx, errMsg, n);
				}
				else {
					return;
				}
			}

			parser.popState();
		}
	};

	ParserOptions options;

	std::deque<State> stateStack;
	language::AST ast;
	

	const lexer::FileContext* currentCtx = nullptr;
	language::CompileFlags currentNodeFlags;

	Parser(const utils::cmd_line_options&);

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

	bool prevStateMatches(const std::vector<State>& states, size_t offsetFromCurrent) const {
		auto it = stateStack.rbegin();
		for (auto i = 0; i < offsetFromCurrent; i++) {
			++it;
			if (it == stateStack.rend()) {
				return false;
			}
		}
		auto it2 = states.rbegin();

		while ((it != stateStack.rend()) && (it2 != states.rend())) {
			if (*it != *it2) {
				return false;
			}

			++it2;
			++it;
		}

		return true;
	}

	bool prevStateMatches(const std::vector<State>& states) const {
		auto it = stateStack.rbegin();
		auto it2 = states.rbegin();

		while (it != stateStack.rend()) {
			if (*it == *it2) {
				break;
			}
			++it;
		}
		

		while ((it != stateStack.rend()) && (it2 != states.rend())) {
			if (*it != *it2) {
				return false;
			}

			++it2;
			++it;
		}

		return true;
	}

	State prevState(size_t count) const {
		auto it = stateStack.rbegin();
		for (auto i = 0; i < count; i++) {
			++it;
			if (it == stateStack.rend()) {
				break;
			}
		}
		
		if (it != stateStack.rend()) {
			return *it;
		}
		else {
			return stateStack.front();
		}

		return parser::NONE;
	}

	void clearState() {
		stateStack.clear();
		pushState(NONE);
	}

	void addCompilerFlagsToNode(language::ParseNode& node) {
		if (!currentNodeFlags.flags.empty()) {
			node.flags = currentNodeFlags;
			currentNodeFlags.flags.clear();
		}
		
		
	}

	void error(const lexer::Token& token, const lexer::FileContext& ctx, const std::string& errMsg) {
		pushState(ERROR);

		throw Parser::Error(token, ctx, errMsg, options.debugMode? stateStack: std::deque<State>());
	}

	void error(const lexer::Token& token, const lexer::FileContext& ctx, const std::string& errMsg, const language::ParseNode* result) {
		delete result;
		popState();

		pushState(ERROR);

		throw Parser::Error(token, ctx, errMsg, options.debugMode ? stateStack : std::deque<State>());;
	}

	bool lookAhead(lexer::Token::Type* types, size_t typesSize, const lexer::FileContext& ctx)
	{
		std::vector<lexer::Token> tokens;
		prevToken(ctx);

		bool res = peekNext(ctx, typesSize, tokens);

		nextToken(ctx, nullptr, false);

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

	void verifyTokenTypeOrFail(const lexer::Token& t, lexer::Token::Type type, const lexer::FileContext& ctx, const std::string& errMsg, const language::ParseNode* node)
	{
		if (t.type != type) {
			error(t, ctx, errMsg, node);
		}
	}

	void verifyTokenTypeOrFail(const lexer::Token& t, lexer::Token::Type* types, size_t typesSize, const lexer::FileContext& ctx, const std::string& errMsg, const language::ParseNode* node)
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

	void verifyTokenOrFail(const lexer::Token& t, lexer::Token::Type type, const std::string& expectedValue, const lexer::FileContext& ctx,
		const std::string& errMsg, language::ParseNode* node)
	{		
		if (t.type != type || t.text != expectedValue) {
			error(t, ctx, errMsg, node);
		}
	}

	bool beginToken(const lexer::FileContext& ctx, language::ParseNode* parent) {
		ctx.beginTokens();

		if (ctx.tokenCount() == 0) {
			return false;
		}

		this->commentCheck(ctx, parent);

		return true;
	}

	lexer::Token peekNext(const lexer::FileContext& ctx) {
		lexer::Token result;

		ctx.peekNextToken(result);

		return result;
	}

	lexer::Token peekNext(const lexer::FileContext& ctx, size_t offset) {
		lexer::Token result;

		ctx.peekNextToken(result, offset);

		return result;
	}


	bool peekNext(const lexer::FileContext& ctx, size_t tokenCount, std::vector<lexer::Token>& tokens) {
				
		return ctx.peekNextTokens(tokenCount, tokens);
	}


	lexer::Token peekPrev(const lexer::FileContext& ctx) {
		lexer::Token result;

		ctx.peekPrevToken(result);

		return result;
	}

	bool nextToken(const lexer::FileContext& ctx, language::ParseNode* parent, bool checkForComments=true) {
		
		if (!ctx.nextToken()) {
			return false;
		}

		if (checkForComments) {
			this->commentCheck(ctx, parent);
		}

		return true;
	}

	bool prevToken(const lexer::FileContext& ctx) {
		return ctx.prevToken();
	}

	ClassBlock* classBlock(const lexer::FileContext& ctx, language::ParseNode* parent) {
		ClassBlock* result = nullptr;
		
		ParseStateGuard pg(*this, CLASS, ctx, "Parsing class statement, invalid state exiting parse", result);


		verifyTokenOrFail(ctx.getCurrentToken(),
			lexer::Token::KEYWORD,
			language::Keywords[language::KEYWORD_CLASS],
			ctx,
			"Parsing class, expected 'class' keyword",
			result);

		result = new ClassBlock();
		result->parent = parent;
		result->token = ctx.getCurrentToken();

		nextToken(ctx, parent);
		auto tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			lexer::Token::IDENTIFIER,
			ctx,
			"Parsing class, expected identifier for name of class",
			result);
		
		result->name = tok.text.str();

		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		lexer::Token::Type types[] = { lexer::Token::KEYWORD, lexer::Token::OPEN_BLOCK};
		verifyTokenTypeOrFail(tok,
			types,sizeof(types)/sizeof(types[0]),
			ctx,
			"Parsing class, expected either keyword ('inherits'), or class open block '{'",
			result);

		if (tok.type == lexer::Token::KEYWORD) {
			if (tok.text == language::Keywords[language::KEYWORD_INHERITS]) {
				nextToken(ctx, parent);
				tok = ctx.getCurrentToken();
				verifyTokenTypeOrFail(tok,
					lexer::Token::IDENTIFIER,
					ctx,
					"Parsing class, expected identifier for super class name '}'",
					result);

				result->super = tok.text.str();
			}

			nextToken(ctx, result);
			
		}

		tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			lexer::Token::OPEN_BLOCK,
			ctx,
			"Parsing class, expected class open block '{'",
			result);


		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		//class should be in place, now build out the innards

		language::CompileFlags currentFlags;

		while (ctx.getCurrentToken().type != lexer::Token::CLOSE_BLOCK) {
			tok = ctx.getCurrentToken();
			if (tok.type == lexer::Token::KEYWORD && 
				(tok.text == language::Keywords[language::KEYWORD_PUBLIC] || 
					tok.text == language::Keywords[language::KEYWORD_PRIVATE])) {
				ScopeNode* scope = new ScopeNode();
				scope->parent = result;
				scope->name = "scope-" + tok.text.str();
				scope->scope = tok.text.str();

				nextToken(ctx, parent);
				tok = ctx.getCurrentToken();

				verifyTokenTypeOrFail(tok,
					lexer::Token::OPEN_BLOCK,
					ctx,
					"Parsing class, expected private/public scope open block '{'",
					result);

				nextToken(ctx, result);
				tok = ctx.getCurrentToken();



				while (ctx.getCurrentToken().type != lexer::Token::CLOSE_BLOCK) {


					VariableNode* memberVar = variableDef(ctx, scope);
					memberVar->parent = scope;

					scope->memberVars.push_back(memberVar);

					nextToken(ctx, scope);

					verifyTokenTypeOrFail(ctx.getCurrentToken(),
						lexer::Token::END_OF_STATEMENT,
						ctx,
						"Parsing class, expected end of statement (';') for public/private member variable definition",
						result);

					nextToken(ctx, scope);
				}


				tok = ctx.getCurrentToken();

				verifyTokenTypeOrFail(tok,
					lexer::Token::CLOSE_BLOCK,
					ctx,
					"Parsing class, expected private/public scope close block '}'",
					result);


				result->scopes.push_back(scope);

				nextToken(ctx, result);
				tok = ctx.getCurrentToken();
			}
			else if (tok.type == lexer::Token::AT_SIGN) {
				currentFlags = language::CompileFlags();
				currentFlags = compilerFlags(ctx, parent);
				nextToken(ctx, result);
			}
			else if (tok.type == lexer::Token::KEYWORD && (tok.text == language::Keywords[language::KEYWORD_MSG]) ) {
				//message declaration and block

				MessageDeclaration* msgDecl = messageDecl(ctx, result);
				msgDecl->flags = currentFlags;

				nextToken(ctx, result);

				verifyTokenTypeOrFail(ctx.getCurrentToken(),
					lexer::Token::OPEN_BLOCK,
					ctx,
					"Parsing class, expected message def open block '{'",
					result);

				MessageBlock* msgBlock = messageBlock(ctx, result);
				msgBlock->name = msgDecl->name;
				msgBlock->msgDecl = msgDecl;

				msgDecl->parent = msgBlock;

				result->messages.push_back(msgBlock);

				nextToken(ctx, result);
			}
		}


		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::CLOSE_BLOCK,
			ctx,
			"Parsing class, expected class close block '}'",
			result);


		return result;
	}



	RecordBlock* recordBlock(const lexer::FileContext& ctx, language::ParseNode* parent) {
		RecordBlock* result = nullptr;

		ParseStateGuard pg(*this, RECORD, ctx, "Parsing record statement, invalid state exiting parse", result);


		verifyTokenOrFail(ctx.getCurrentToken(),
			lexer::Token::KEYWORD,
			language::Keywords[language::KEYWORD_RECORD],
			ctx,
			"Parsing record, expected 'record' keyword",
			result);

		result = new RecordBlock();
		result->parent = parent;

		nextToken(ctx, parent);
		auto tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			lexer::Token::IDENTIFIER,
			ctx,
			"Parsing record, expected identifier for name of record",
			result);

		result->name = tok.text.str();

		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		lexer::Token::Type types[] = { lexer::Token::KEYWORD, lexer::Token::OPEN_BLOCK };
		verifyTokenTypeOrFail(tok,
			types, sizeof(types) / sizeof(types[0]),
			ctx,
			"Parsing record, expected either keyword ('inherits'), or class open block '{'",
			result);

		if (tok.type == lexer::Token::KEYWORD) {
			if (tok.text == language::Keywords[language::KEYWORD_INHERITS]) {
				nextToken(ctx, parent);
				tok = ctx.getCurrentToken();
				verifyTokenTypeOrFail(tok,
					lexer::Token::IDENTIFIER,
					ctx,
					"Parsing record, expected identifier for super class name '}'",
					result);

				result->super = tok.text.str();
			}

			nextToken(ctx, result);

		}

		tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			lexer::Token::OPEN_BLOCK,
			ctx,
			"Parsing record, expected record open block '{'",
			result);


		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		//class should be in place, now build out the innards

		while (ctx.getCurrentToken().type != lexer::Token::CLOSE_BLOCK) {
			VariableNode* memberVar = variableDef(ctx, result);
			memberVar->parent = result;

			result->memberVars.push_back(memberVar);

			nextToken(ctx, result);

			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				lexer::Token::END_OF_STATEMENT,
				ctx,
				"Parsing record, expected end of statement (';') for member variable definition",
				result);


			nextToken(ctx, result);
			tok = ctx.getCurrentToken();
		}


		verifyTokenTypeOrFail(tok,
			lexer::Token::CLOSE_BLOCK,
			ctx,
			"Parsing record, expected record close block '}'",
			result);


		return result;
	}

	

	language::ParseNode* returnExpression(const lexer::FileContext& ctx, const language::ParseNode* parent) {
		ReturnExpression* result = nullptr;

		ParseStateGuard pg(*this, RETURN_EXPRESSION, ctx, "Parsing return statement, invalid state exiting parse", result);

		result = new ReturnExpression();
		result->parent = parent;
		nextToken(ctx, result);
		result->retVal = expression(ctx, result);

		return result;
	}

	VariableNode* variableDef(const lexer::FileContext& ctx, language::ParseNode* parent) {
		VariableNode* result = nullptr;

		ParseStateGuard pg(*this, VARIABLE_DEFINITION, ctx, "Parsing variable definition, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::IDENTIFIER,
			ctx,
			"Parsing variable, expected identifier",
			result);

		auto tok = ctx.getCurrentToken();

		std::string varName = parseVariableName(ctx, parent);

		result = new VariableNode();
		result->parent = parent;
		result->token = tok;
		result->name = varName;

		nextToken(ctx, parent);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::COLON,
			ctx,
			"Parsing variable definition, expected colon",
			result);
		

		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			lexer::Token::IDENTIFIER,
			ctx,
			"Parsing variable, expected identifier for type",
			result);


		result->type = tok.text.str();

		


		return result;
	
	}


	std::string typeDeclaration(const lexer::FileContext& ctx, language::ParseNode* parent) {
		std::string result = "";
		ParseStateGuard pg(*this, TYPE_DECLARATION, ctx, "Parsing message param declaration, invalid state exiting parse", nullptr);

		while (nextToken(ctx, parent)) {
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				lexer::Token::IDENTIFIER,
				ctx,
				"Parsing type declaration, expected identifier",
				parent);

			result += ctx.getCurrentToken().text.str();

			if (peekNext(ctx).type == lexer::Token::DOT) {
				nextToken(ctx, parent);
			}
			else {
				break;
			}
		}

		return result;
	}



	ParamNode* messageParamDecl(const lexer::FileContext& ctx, const language::ParseNode* parent) {
		ParamNode* result = nullptr;
		ParseStateGuard pg(*this, PARAM_DECLARATION, ctx, "Parsing message param declaration, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::IDENTIFIER,
			ctx,
			"Parsing param declaration, identifier expected",
			result);

		auto paramId = ctx.getCurrentToken();

		nextToken(ctx, result);
		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::COLON,
			ctx,
			"Parsing param declaration, expected ':'",
			result);


		result = new ParamNode();
		result->parent = parent;
		result->name = paramId.text.str();
		result->token = paramId;
		std::string type;

		type = typeDeclaration(ctx, result);
		
		result->type = type;
		if (type.empty()) {
			delete result;
			result = nullptr;
		}

		return result;
	}

	TupleNode* returnDeclaration(const lexer::FileContext& ctx, const language::ParseNode* parent) {
		TupleNode* result = nullptr;
		ParseStateGuard pg(*this, RETURN_DECLARATION, ctx, "Parsing return declaration, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::OPEN_BRACKET,
			ctx,
			"Parsing return declaration, expected '['",
			result);

		result = new TupleNode();
		result->parent = parent;
		result->name = "return";
		result->token = ctx.getCurrentToken();
		std::string type = "";
		
		if (peekNext(ctx).type != lexer::Token::CLOSE_BRACKET) {
			do {
				type = typeDeclaration(ctx, result);
				if (!type.empty()) {
					result->fields.push_back(type);
					auto nextT = peekNext(ctx);
					if (nextT.type == lexer::Token::CLOSE_BRACKET) {
						break;
					}
					else if (nextT.type == lexer::Token::COMMA) {
						nextToken(ctx, result);
					}
				}
			} while (!type.empty());
		}		

		nextToken(ctx, result);
		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::CLOSE_BRACKET,
			ctx,
			"Parsing return declaration, expected ']'",
			result);

		return result;
	}

	language::ParseNode* variable(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, VARIABLE, ctx, "Parsing variable, invalid state exiting parse", result);
		lexer::Token::Type types[] = { lexer::Token::IDENTIFIER, lexer::Token::KEYWORD };

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			types, sizeof(types)/sizeof(types[0]),
			ctx,
			"Parsing variable, expected identifier or nil",
			result);

		auto tok = ctx.getCurrentToken();

		lexer::Token::Type types2[] = { lexer::Token::IDENTIFIER, lexer::Token::COLON, lexer::Token::IDENTIFIER};
		
		if (lookAhead(types2, sizeof(types2) / sizeof(types2[0]), ctx)) {
			result = variableDef(ctx, parent);			
		}
		else if (tok.type == lexer::Token::KEYWORD ) {
			if (tok.isNil()) {
				NilNode* nilInst = new NilNode();
				nilInst->parent = parent;
				result = nilInst;
				result->token = tok;
			}
			else {
				error(tok, ctx, "Parsing variable, expecting 'nil' keyword, but found something else", result);
			}
		}
		else {
			VariableNode* varNode = new VariableNode();
			varNode->parent = parent;
			varNode->name = tok.text.str();
			result = varNode;
			result->token = tok;
		}

		return result;
	}


	language::ParseNode* groupedExpression(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, GROUPED_EXPRESSION, ctx, "Parsing group expression, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::OPEN_PAREN,
			ctx,
			"Parsing grouped expression, expected closing paren ')'",
			result);
		auto tok = ctx.getCurrentToken();
		nextToken(ctx, parent);

		GroupedExpression* grpExpr = new GroupedExpression();

		grpExpr->parent = parent;

		language::ParseNode* expr = expression(ctx, grpExpr);

		grpExpr->innerExpression = expr;//expressions.push_back(expr);

		result = grpExpr; 
		result->token = tok;

		if (ctx.getCurrentToken().type != lexer::Token::CLOSE_PAREN &&
			peekNext(ctx).type == lexer::Token::CLOSE_PAREN) {
			nextToken(ctx, parent);
		}

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::CLOSE_PAREN,
			ctx,
			"Parsing grouped expression, expected closing paren ')'",
			result);

		return result;
	}


	language::ParseNode* buildLhsExpression(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;
		auto first = ctx.getCurrentToken();

		lexer::Token second;
		size_t peekOffset = 0;

		if (first.isLiteral()) {
				result = instance(ctx, parent);
		}
		else {
			switch (first.type) {
				case lexer::Token::IDENTIFIER: {
					second = peekNext(ctx, peekOffset);

					if (second.isOperator()) {
						result = instance(ctx, parent);
					}
					else {
						switch (second.type) {
							case lexer::Token::DOT: {
								result = instance(ctx, parent);
							}
							break;

							case lexer::Token::IDENTIFIER: {
								result = instance(ctx, parent);
							}
							break;

							case lexer::Token::COLON: {
								result = variableDef(ctx, parent);
							}
														 break;

							default: {
								error(second, ctx, "building LHS, invalid code, expecting '.', operator, or method name", result);
							} break;
						}
					}
					

				} break;

				case lexer::Token::OPEN_PAREN: {
					result = expression(ctx, parent);
				}
				break;

				default: {
					error(first, ctx, "building LHS, invalid code", result);
				} break;
			}
		}

		
		return result;
	}

	void makeMessageParam(const lexer::FileContext& ctx, Message* parent, const std::string& paramName, language::ParseNode* instance) {
		

		auto msgParam = new MessageParam();
		msgParam->parent = parent;
		instance->parent = msgParam;
		msgParam->param = instance;

		if (!paramName.empty()) {
			msgParam->name = paramName;
		}
		parent->parameters.push_back(msgParam);
	}

	void buildRhsExpression(const lexer::FileContext& ctx, Message* parent, const lexer::Token& msgNameTok) {
		
		auto tok = ctx.getCurrentToken();
		lexer::Token second;
		size_t peekOffset = 0;

		if (!msgNameTok.isOperator()) {
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				lexer::Token::COLON,
				ctx,
				"Parsing grouped expression, expected ':'",
				parent);

			nextToken(ctx, parent);

			tok = ctx.getCurrentToken();
		}

		

		bool done = false;

		std::string paramName;
		bool firstParam = true;
		bool parseInstance = false;
		bool parseExpression = false;
		bool doPrevToken = false;
		bool addParam = false;

		language::ParseNode* instanceNode = nullptr;
		language::ParseNode* groupExpr = nullptr;
		while (!done) {
			if (tok.isLiteral()) {
				parseInstance = true;
				parseExpression = false;							
			}
			else {
				switch (tok.type) {
					case lexer::Token::IDENTIFIER: {
						
						if (!firstParam && paramName.empty() && peekNext(ctx).type != lexer::Token::COLON ) {
							error(ctx.getCurrentToken(), ctx, "param name not found, invalid param definition");
						}
						
						parseInstance = true;
						parseExpression = false;
						
					} break;

					case lexer::Token::KEYWORD: {
						if (tok.isNil()) {
							parseInstance = true;
							parseExpression = false;
						}
						else if (tok.text == language::Keywords[language::KEYWORD_MSG]) {
							parseInstance = true;
							parseExpression = false;
						}
					} break;

					case lexer::Token::AT_SIGN: {
						nextToken(ctx, parent);
						tok = ctx.getCurrentToken();
						verifyTokenTypeOrFail(tok, lexer::Token::IDENTIFIER,
							ctx, "Parsing message param, expected identifier for param name",
							parent);

						paramName = tok.text.str();
						nextToken(ctx, parent);
						tok = ctx.getCurrentToken();

						verifyTokenTypeOrFail(tok, lexer::Token::COLON,
							ctx, "Parsing message param, expected ':' following param name",
							parent);
					} break;

					case lexer::Token::END_OF_STATEMENT: {
						done = true;
						doPrevToken = true;
					} break;

					case lexer::Token::OPEN_PAREN: {
						parseInstance = true;
						parseExpression = true;
						doPrevToken = true;
					} break;  

					case lexer::Token::CLOSE_PAREN: {
						done = true;
						//doPrevToken = true;
					} break;

					default: {
						error(tok, ctx, "building RHS, invalid code", parent);
					} break;
				}
			}

			if (!done) {

				if (parseInstance) {

					

					auto nextTok = peekNext(ctx);

					if (nextTok.isMathOperator()) {
						parseExpression = true;
						//mathematical operator like +/-* etc, special treatment
					}
					else if (nextTok.type == lexer::Token::IDENTIFIER) {
						parseExpression = true;
						//identifier 
					}
					else if (tok.type == lexer::Token::IDENTIFIER && 
						nextTok.type == lexer::Token::COLON) {
						parseExpression = true;
						prevToken(ctx);
						//method name
					}


					if (parseExpression) {
						//auto lhs = instanceNode;

						if (nullptr != groupExpr) {
							instanceNode = expression(ctx, parent, groupExpr);
							groupExpr = nullptr;
						}
						else {
							instanceNode = expression(ctx, parent);
							groupExpr = nullptr;
						}

						
						
						
						addParam = true;
					}
					else {
						instanceNode = instance(ctx, parent);
						addParam = true;
					}		
				}

				if (addParam && nullptr != instanceNode) {
					makeMessageParam(ctx, parent, firstParam ? "" : paramName, instanceNode);
					
					addParam = false;


					paramName = "";
					firstParam = false;
					parseExpression = false;
					parseInstance = false;

					if (ctx.getCurrentToken().type == lexer::Token::CLOSE_PAREN) {
						done = true;

						if (instanceNode->token.type == lexer::Token::OPEN_PAREN) {
							groupExpr = instanceNode;
							done = false;
							doPrevToken = false;
						}
					}
					else if (ctx.getCurrentToken().type == lexer::Token::END_OF_STATEMENT) {
						done = true;
						doPrevToken = true;
					}

					instanceNode = nullptr;
				}
				

				if (doPrevToken) {
					prevToken(ctx);
				}

				if (!done) {
					nextToken(ctx, parent);
					tok = ctx.getCurrentToken();
				}
			}
			else {
				if (doPrevToken) {
					prevToken(ctx);
				}
			}
			doPrevToken = false;
		}
	}


	language::ParseNode* expression(const lexer::FileContext& ctx, language::ParseNode* parent, language::ParseNode* lhs=nullptr) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, EXPRESSION, ctx, "Parsing expression, invalid state exiting parse", result);


		auto first = ctx.getCurrentToken();
		if (first.type == lexer::Token::KEYWORD) {
			if (first.text == language::Keywords[language::KEYWORD_RETURN]) {
				result = returnExpression(ctx, parent);
			}
		}
		else if (first.type == lexer::Token::OPEN_PAREN) {
			//should be expression!
			result = groupedExpression(ctx, parent);
		}
		else {
			lexer::Token sendMessageTok;
			if (nullptr == lhs) {
				lhs = buildLhsExpression(ctx, parent);
				sendMessageTok = ctx.getCurrentToken();
			}
			else {
				sendMessageTok = peekNext(ctx);
			}

			result = sendMessage(ctx, parent, lhs);
			result->token = sendMessageTok;
		}
		/*
		
		*/
		return result;
	}

	std::string parseVariableName(const lexer::FileContext& ctx, language::ParseNode* parent)
	{
		std::string result;		
		auto tok = ctx.getCurrentToken();
		auto instTok = tok;
		result = tok.text.str();

		auto nextTok = peekNext(ctx);
		if (nextTok.type == lexer::Token::DOT) {

			result = tok.text.str();
			nextToken(ctx, parent);//should be DOT
			nextToken(ctx, parent);//next component...
			tok = ctx.getCurrentToken();
			while (tok.type == lexer::Token::IDENTIFIER && peekNext(ctx).type == lexer::Token::DOT) {
				result += "." + tok.text.str();
				nextToken(ctx, parent);//should be DOT
				nextToken(ctx, parent);//next component...
				tok = ctx.getCurrentToken();
			}
			if (tok.type == lexer::Token::IDENTIFIER && peekNext(ctx).type != lexer::Token::DOT) {
				result += "." + tok.text.str();
			}
		}

		return result;
	}

	language::ParseNode* msgInstance(const lexer::FileContext& ctx, language::ParseNode* parent)
	{
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, MSG_INSTANCE, ctx, "Parsing msg instance, invalid state exiting parse", result);

		auto nextT = peekNext(ctx);
		auto prevT = peekPrev(ctx);

		
		
		verifyTokenTypeOrFail(nextT,
			lexer::Token::OPEN_PAREN,
			ctx,
			"Parsing msg instance, expected  '('",
			result);


		verifyTokenTypeOrFail(prevT,
			lexer::Token::ASSIGMENT_OPERATOR,
			ctx,
			"Parsing msg instance, expected  ':='",
			result);

		MsgInstanceNode* msgInst = new MsgInstanceNode();

		auto msgDecl = messageDecl(ctx, msgInst);

		auto tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			lexer::Token::OPEN_BLOCK,
			ctx,
			"Parsing msg instance block, expected  '{'",
			result);

		auto msgBlock = messageBlock(ctx, msgInst);
		msgBlock->msgDecl = msgDecl;

		//msgInst->msgDecl = msgDecl;
		msgInst->msgBlock = msgBlock;

		result = msgInst;

		return result;
	}

	language::ParseNode* instance(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, INSTANCE, ctx, "Parsing instance, invalid state exiting parse", result);

		auto tok = ctx.getCurrentToken();

		if (tok.isLiteral()) {
			language::ParseNode* varLit = varLiteral(ctx, parent);		

			result = varLit;
			result->token = tok;
		}
		else if (tok.type == lexer::Token::IDENTIFIER) {
			auto instTok = tok;
			std::string varName = parseVariableName(ctx, parent);

			InstanceNode* instance = new InstanceNode();
			instance->parent = parent;
			instance->name = varName;
			result = instance;
			result->token = instTok;
		}
		else if (tok.type == lexer::Token::KEYWORD) {
			if (tok.isNil()) {
				NilNode* nilInst = new NilNode();
				nilInst->parent = parent;
				result = nilInst;
				result->token = tok;
			}
			else if (tok.text == language::Keywords[language::KEYWORD_MSG]) {
				result = msgInstance(ctx, parent);
				result->token = tok;
			}
			else {
				result = nullptr;
				error(tok, ctx, "Parsing instance, expecting 'nil' keyword, but found something else", result);
			}
		}
		

		return result;
	}

	SendMessage* sendMessage(const lexer::FileContext& ctx, const language::ParseNode* parent, language::ParseNode* instance) {
		SendMessage* result = nullptr;

		ParseStateGuard pg(*this, SEND_MESSAGE, ctx, "Parsing send message, invalid state exiting parse", result);
		
		result = new SendMessage();
		result->parent = parent;
		result->token = ctx.getCurrentToken();

		auto tok = ctx.getCurrentToken();

		result->instance = instance;
		
		instance->parent = result;

		nextToken(ctx, result);
		
		auto msg = message(ctx, result);
		result->message = msg;

		return result;
	}

	language::ParseNode* assignment(const lexer::FileContext& ctx, const language::ParseNode* parent) {
		language::ParseNode* result = nullptr;
		
		Assignment* assignmentNode = new Assignment();
		assignmentNode->parent = parent;

		ParseStateGuard pg(*this, ASSIGNMENT, ctx, "Parsing assignment, invalid state exiting parse", result);

		result = assignmentNode;
		result->token = ctx.getCurrentToken();


		auto tokIdent = ctx.getCurrentToken();

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::IDENTIFIER,
			ctx,
			"Parsing assignment, expected identier for variable/instance name",
			result);

		nextToken(ctx, assignmentNode);

		auto tok = ctx.getCurrentToken();

		if (tok.type == lexer::Token::COLON) {
			prevToken(ctx);
			language::ParseNode* var = variable(ctx, assignmentNode);
			var->parent = assignmentNode;

			assignmentNode->instance = var;

		}
		else {
			prevToken(ctx);
			//expression?
			language::ParseNode* inst = instance(ctx, assignmentNode);
				/*new InstanceNode();
			instance->parent = assignmentNode;
			instance->name = tokIdent.text.str();
			*/
			assignmentNode->instance = inst;

		}

		nextToken(ctx, assignmentNode);

		auto assignmentOpMsg = message(ctx, assignmentNode);

		assignmentNode->message = assignmentOpMsg;

		return result;
	}


	language::ParseNode* varLiteral(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		auto tok = ctx.getCurrentToken();
		switch (tok.type) {
			case lexer::Token::OPEN_BRACKET: {
				bool arrayClosed = false;
				ArrayLiteralNode* arrayLit = new ArrayLiteralNode();
				arrayLit->parent = parent;
				arrayLit->token = tok;
				while (nextToken(ctx,parent) && !arrayClosed) {
					tok = ctx.getCurrentToken();

					if (tok.isLiteral()) {
						language::ParseNode* lit = varLiteral(ctx, arrayLit);

						arrayLit->elements.push_back(lit);
					}
					else {
						switch (tok.type) {
							case lexer::Token::CLOSE_BRACKET: {
								arrayClosed = true;
								break; //loop
							}
							break;

							case lexer::Token::END_OF_STATEMENT: {
									
								break; //loop
							}
							break;

							case lexer::Token::IDENTIFIER: {
								auto inst = instance(ctx, arrayLit);
								arrayLit->elements.push_back(inst);
							}
							break;

							case lexer::Token::COMMA: {
								
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

				if (ctx.getCurrentToken().type == lexer::Token::END_OF_STATEMENT) {
					prevToken(ctx);
				}

				result = arrayLit;
			} break;

			case lexer::Token::INTEGER_LITERAL: case lexer::Token::BOOLEAN_LITERAL:
			case lexer::Token::BINARY_LITERAL: case lexer::Token::DECIMAL_LITERAL:
			case lexer::Token::HEXADECIMAL_LITERAL: case lexer::Token::STRING_LITERAL: {
				LiteralNode* literal = new LiteralNode();
				literal->parent = parent;
				literal->val = tok.text.str();
				literal->token = tok;
				switch (tok.type) {
					case lexer::Token::INTEGER_LITERAL:literal->type = LiteralNode::INTEGER_LITERAL;break;
					case lexer::Token::BOOLEAN_LITERAL:literal->type = LiteralNode::BOOLEAN_LITERAL;break;
					case lexer::Token::BINARY_LITERAL:literal->type = LiteralNode::BINARY_LITERAL;break;
					case lexer::Token::HEXADECIMAL_LITERAL:literal->type = LiteralNode::HEXADECIMAL_LITERAL;break;
					case lexer::Token::DECIMAL_LITERAL:literal->type = LiteralNode::DECIMAL_LITERAL;break;
					case lexer::Token::STRING_LITERAL:literal->type = LiteralNode::STRING_LITERAL;break;
				}
				result = literal;

			} break;
		}
		return result;
	}

	MessageDeclaration* messageDecl(const lexer::FileContext& ctx, language::ParseNode* parent) {
		MessageDeclaration* result = nullptr;

		ParseStateGuard pg(*this, MESSAGE_DECL, ctx, "Parsing message declaration, invalid state exiting parse", result);

		verifyTokenOrFail(ctx.getCurrentToken(),
			lexer::Token::KEYWORD,
			language::Keywords[language::KEYWORD_MSG],
			ctx,
			"Parsing message, expected 'msg' keyword",
			result);

		

		nextToken(ctx, parent);
		auto tok = ctx.getCurrentToken();

		
		lexer::Token::Type types[] = { lexer::Token::IDENTIFIER, lexer::Token::OPEN_PAREN };

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			types, sizeof(types) / sizeof(types[0]),
			ctx,
			"Parsing message, identier for msg name or '(' for closure msg",
			result);

		auto msgName = ctx.getCurrentToken();

		bool closureMessage = false;

		result = new MessageDeclaration();
		result->parent = parent;
		result->token = msgName;

		if (ctx.getCurrentToken().type == lexer::Token::IDENTIFIER) {
			result->name = msgName.text.str();
		}
		else {
			result->name = "";
			closureMessage = true;
		}
		
		if (!closureMessage) {
			nextToken(ctx, result);
		}

		result->msgIsClosure = closureMessage;
		
		tok = ctx.getCurrentToken();
		if ((tok.type == lexer::Token::COLON) || (tok.type == lexer::Token::OPEN_PAREN)) {
			//params
			nextToken(ctx, result);
			ParamNode* param = nullptr;
			do {
				tok = ctx.getCurrentToken();
				if (tok.type == lexer::Token::OPEN_BLOCK ) {  //start of message block
					ctx.prevToken(); //rewind one
					break;
				}
				else if (tok.type == lexer::Token::OPEN_BRACKET) { //start of return type
					break;
				}
				else if (tok.type == lexer::Token::CLOSE_PAREN) { //end of decl for closure
					nextToken(ctx, result);
					break;
				}

				param = messageParamDecl(ctx, result);
				if (param) {
					result->params.push_back(param);

					nextToken(ctx, result);
					tok = ctx.getCurrentToken();
					if (tok.type == lexer::Token::COMMA) {
						nextToken(ctx, result);
					}
				}				
			} while (nullptr != param);
		}

		tok = ctx.getCurrentToken();
		
		if (tok.type == lexer::Token::OPEN_BRACKET) {
			TupleNode* returnDecl = returnDeclaration(ctx, result);
			result->returnType = returnDecl;

			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				lexer::Token::CLOSE_BRACKET,
				ctx,
				"Parsing message, expected 'identifier ']' for msg return type declaration",
				result);
		}

		return result;
	}

	MessageBlock* messageBlock(const lexer::FileContext& ctx, const language::ParseNode* parent) {
		MessageBlock* result = nullptr;

		ParseStateGuard pg(*this, MESSAGE_BLOCK, ctx, "Parsing message block, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::OPEN_BLOCK,
			ctx,
			"Parsing message block, expected '{' character",
			result);

		result = new MessageBlock();
		result->parent = parent;
		
		nextToken(ctx, result);

		if (ctx.getCurrentToken().type != lexer::Token::CLOSE_BLOCK) {
			StatementsBlock* msgStatements = statements(ctx, result);
			if (msgStatements) {
				result->statements = msgStatements;
			}
		}

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::CLOSE_BLOCK,
			ctx,
			"Parsing message block, expected '}' character",
			result);

		return result;
	}

	language::ParseNode* messageParamDecl(const lexer::FileContext& ctx, language::ParseNode* parent, bool firstParam) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, MESSAGE_PARAM, ctx, "Parsing message param, invalid state exiting parse", result);

		auto tok = ctx.getCurrentToken();
		std::string paramName = "";

		if (!firstParam) {
			//extract message name
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				lexer::Token::IDENTIFIER,
				ctx,
				"Parsing message param, expected name of parameter",
				result);

			paramName = ctx.getCurrentToken().text.str();

			nextToken(ctx, result);

			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				lexer::Token::COLON,
				ctx,
				"Parsing message param, expected colon character ':'",
				result);

			nextToken(ctx, result);
			tok = ctx.getCurrentToken();
		}

		switch (tok.type) {
			case lexer::Token::IDENTIFIER: {
				language::ParseNode* var = instance(ctx, parent);// variable(ctx, result);
				var->name = paramName;
				result = var;
			} break;

			case lexer::Token::KEYWORD: {
				if (tok.isNil()) {
					language::ParseNode* var = instance(ctx, parent);
					var->name = paramName;
					result = var;
				}
				else if (tok.text == language::Keywords[language::KEYWORD_MSG]) {

					MessageDeclaration* msgDecl = messageDecl(ctx, parent);
					//msgDecl->flags = currentFlags;

					nextToken(ctx, result);

					verifyTokenTypeOrFail(ctx.getCurrentToken(),
						lexer::Token::OPEN_BLOCK,
						ctx,
						"Parsing class, expected message def open block '{'",
						result);

					MessageBlock* msgBlock = messageBlock(ctx, parent);
					msgBlock->name = paramName.empty() ? msgDecl->name : paramName;
					msgBlock->msgDecl = msgDecl;

					msgDecl->parent = msgBlock;
					
					result = msgBlock;
				}
				else {
					error(ctx.getCurrentToken(), ctx, "Parsing message, expecting 'nil' or message object", result);
				}

			} break;

			case lexer::Token::OPEN_PAREN: {
				language::ParseNode* expr = expression(ctx, parent);
				expr->name = paramName;
				result = expr;
			} break;


			case lexer::Token::OPEN_BRACKET: case lexer::Token::INTEGER_LITERAL:
			case lexer::Token::BOOLEAN_LITERAL:
			case lexer::Token::BINARY_LITERAL: case lexer::Token::DECIMAL_LITERAL:
			case lexer::Token::HEXADECIMAL_LITERAL: case lexer::Token::STRING_LITERAL: {
				language::ParseNode* literal = varLiteral(ctx, parent);
				literal->name = paramName;
				result = literal;

			} break;

			default: {
				result = nullptr;
				error(ctx.getCurrentToken(), ctx, "Parsing message instance, unhandled token", result);
			}
		}
		return result;
	}

	Message* message(const lexer::FileContext& ctx, const language::ParseNode* parent) {
		Message* result = nullptr;

		ParseStateGuard pg(*this, MESSAGE, ctx, "Parsing message, invalid state exiting parse", result);

		lexer::Token::Type types []  = { lexer::Token::IDENTIFIER,
								lexer::Token::ASSIGMENT_OPERATOR,
								lexer::Token::ADDITION_OPERATOR,
								lexer::Token::MULT_OPERATOR,
								lexer::Token::SUBTRACTION_OPERATOR,
								lexer::Token::DIV_OPERATOR,
								lexer::Token::MOD_OPERATOR,
								lexer::Token::OPEN_PAREN,
								lexer::Token::AT_SIGN };

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			types,sizeof(types)/sizeof(lexer::Token::Type),
			ctx,
			"Parsing message, expected identier",
			result);

		auto msgName = ctx.getCurrentToken();
		
		result = new Message();
		result->parent = parent;
		result->name = msgName.text.str();
		result->token = ctx.getCurrentToken();

		nextToken(ctx, result);
		
		buildRhsExpression(ctx, result, msgName);
		

		return result;
	}

	language::ParseNode* instanceExpr(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, INSTANCE_EXPR, ctx, "Parsing instance expr, invalid state exiting parse", result);
		//instance - expr :: == variable_id | instance_var | variable_def | var_literal | expression

		auto first = ctx.getCurrentToken();

		if (first.type == lexer::Token::AT_SIGN) {

			currentNodeFlags = compilerFlags(ctx, parent);
			nextToken(ctx, parent);

			first = ctx.getCurrentToken();
		}


		return result;
	}

	

	language::ParseNode* statement(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, STATEMENT, ctx, "Parsing statement, invalid state exiting parse", result);
		auto first = ctx.getCurrentToken();

		if (first.type == lexer::Token::AT_SIGN) {

			currentNodeFlags = compilerFlags(ctx, parent);
			nextToken(ctx, parent);

			first = ctx.getCurrentToken();
		}
		//  1             2           3            4       
		//variable_def | assignment| expression | return-expression    end_of_statement

		bool emptyBlock = false;

		switch (first.type) {
			case lexer::Token::IDENTIFIER: {
				//options - 
				// 1.1 identifier ':' identifier  ';'  ==>> variable_def
				// 1.2 identifier ':' (identifier '.')+ identifier  ';'  ==>> variable_def
				// 1.3 (identifier '.') + identifier ':' identifier  ';'  ==>> variable_def
				// 1.4 (identifier '.')+ identifier ':' (identifier '.')+ identifier  ';'  ==>> variable_def
				

				ctx.saveCurrentToken();

				
				nextToken(ctx, parent);
				if (ctx.getCurrentToken().type == lexer::Token::COLON) {  //probably 1 or 2
					ctx.restoreCurrentToken();
					result = variableDef(ctx, parent);
				}
				else {
					ctx.restoreCurrentToken();
					result = expression(ctx, parent);
				}
			}
			break;


			case lexer::Token::KEYWORD : { //return keyword! 
				if (first.text == language::Keywords[language::KEYWORD_RETURN]) {
					result = returnExpression(ctx, parent);
				}
				else {
					error(ctx.getCurrentToken(), ctx, "Parsing statement, found keyword, expecting 'return'", result);
				}

			}break;

			case lexer::Token::OPEN_PAREN: { //group! let expression handle it				
				result = expression(ctx, parent);
			}break;


			case lexer::Token::CLOSE_BLOCK: { //group! let expression handle it				
				if (nullptr == result) {
					auto prevTok = this->peekPrev(ctx);
					if (prevTok.type == lexer::Token::OPEN_BLOCK) {
						emptyBlock = true;
					}
				}
			}break;
			
			default: {
				error(ctx.getCurrentToken(), ctx, "Parsing statement, invalid code", result);
			} break;
		}


		if (!emptyBlock) {
			nextToken(ctx, result);
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				lexer::Token::END_OF_STATEMENT,
				ctx,
				"Parsing statement, expected end of statement: ';'",
				result);
		}

		currentNodeFlags.flags.clear();

		return result;
	}

	language::ParseNode* statementOrCommentOrClass(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;
		auto first = ctx.getCurrentToken();

		if (first.type == lexer::Token::KEYWORD && first.text == language::Keywords[language::KEYWORD_CLASS]) {
			result = classBlock(ctx, parent);
		}
		else if (first.type == lexer::Token::KEYWORD && first.text == language::Keywords[language::KEYWORD_RECORD]) {
			result = recordBlock(ctx, parent);
		}
		else {
			result = statement(ctx, parent);
		}
		

		return result;
	}

	NamespaceBlock* namespaceBlock(const lexer::FileContext& ctx, language::ParseNode* parent) {
		NamespaceBlock* result = nullptr;

		ParseStateGuard pg(*this, NAMESPACE_BLOCK, ctx, "Parsing namespace, invalid state exiting parse", result);

		if (ctx.getCurrentToken().type != lexer::Token::KEYWORD &&
			ctx.getCurrentToken().text != language::Keywords[language::KEYWORD_NAMESPACE]) {
			return result;
		}
						
		verifyTokenOrFail(ctx.getCurrentToken(), 
			lexer::Token::KEYWORD,
			language::Keywords[language::KEYWORD_NAMESPACE],
			ctx, 
			"Parsing namespace, expected namespace keyword", 
			result);

		nextToken(ctx,parent);

		auto& namespaceIdToken = ctx.getCurrentToken();		

		nextToken(ctx, parent);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::OPEN_BLOCK,
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
		if (nextTok.type == lexer::Token::KEYWORD && nextTok.text == language::Keywords[language::KEYWORD_NAMESPACE]) {
			NamespaceBlock* childNamespace = namespaceBlock(ctx, result);
			result->namespaces.push_back(childNamespace);
			if (ctx.getCurrentToken().type == lexer::Token::CLOSE_BLOCK) {
				checkForClose = false;
			}
		}
		else if (nextTok.type == lexer::Token::CLOSE_BLOCK ) {
			checkForClose = false;
		}
		else { //expecting statements
			result->statements = statements(ctx, result);
		}
		
		if (checkForClose) {
			nextToken(ctx, result);
		}	

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::CLOSE_BLOCK,
			ctx,
			"Parsing namespace, expected closing brace '}'",
			result);

		
		return result;
	}

	StatementsBlock* statements(const lexer::FileContext& ctx, language::ParseNode* parent)
	{
		StatementsBlock* result = new StatementsBlock();
		result->parent = parent;
		
		ParseStateGuard pg(*this, STATEMENTS_BLOCK, ctx, "Parsing statements, invalid state exiting parse", result);


		auto statementBlock = statementOrCommentOrClass(*currentCtx, result);
		while (statementBlock != nullptr) {
			result->statements.push_back(statementBlock);

			nextToken(*currentCtx, statementBlock);

			auto& tok = currentCtx->getCurrentToken();
			if (tok.type == lexer::Token::CLOSE_BLOCK) {
				break;
			}

			statementBlock = statementOrCommentOrClass(*currentCtx, result);
		}


		return result;
	}

	language::CompileFlags compilerFlags(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::CompileFlags flags;

		ParseStateGuard pg(*this, COMPILER_FLAGS, ctx, "Parsing compiler flags, invalid state exiting parse", parent);

		
		auto tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			lexer::Token::AT_SIGN,
			ctx,
			"Expected '@' to start flags",
			nullptr);

		nextToken(ctx, parent);
		tok = ctx.getCurrentToken();
		if (tok.type == lexer::Token::OPEN_BLOCK) {
			return flags;
		}
		else {
			verifyTokenTypeOrFail(tok,
				lexer::Token::OPEN_BRACKET,
				ctx,
				"Expected '[' to start flags",
				nullptr);

			
			std::string curFlag;
			while (nextToken(ctx, parent)) {
				
				tok = ctx.getCurrentToken();
				if (tok.type == lexer::Token::IDENTIFIER) {
					curFlag = tok.text.str();
				}
				else if (tok.type == lexer::Token::COMMA) {
					flags.flags.push_back(curFlag);
					curFlag = "";
				}
				else if (tok.type == lexer::Token::CLOSE_BRACKET) {
					if (!curFlag.empty()) {
						flags.flags.push_back(curFlag);
					}
					
					break;
				}
				else {
					error(ctx.getCurrentToken(), ctx, "Parsing compiler flags, invalid syntax", nullptr);
				}
			}
			
			

		}

		return flags;
	}

	CodeFragmentBlock* codeFragmentBlock(const lexer::FileContext& ctx, language::ParseNode* parent) {
		CodeFragmentBlock* result = nullptr;

		ParseStateGuard pg(*this, CODE_FRAGMENT_BLOCK, ctx, "Parsing code fragment, invalid state exiting parse", parent);

		lexer::Token::Type types[] = { lexer::Token::OPEN_BLOCK, lexer::Token::AT_SIGN };

		auto tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			types,sizeof(types)/sizeof(types[0]),
			ctx,
			"Code Fragment Expected open block '{' or '@' for flags",
			result);

		verifyTokenTypeOrFail(ctx.lastToken(),
			lexer::Token::CLOSE_BLOCK,
			ctx,
			"Code Fragment Expected close block '}'",
			result);
		

		CodeFragmentBlock* block = new CodeFragmentBlock();
		result = block;
		

		if (tok.type == lexer::Token::AT_SIGN) {
			//process flags
			currentNodeFlags = compilerFlags(ctx, block);
			addCompilerFlagsToNode(*block);
			nextToken(ctx, block);

			tok = ctx.getCurrentToken();
			verifyTokenTypeOrFail(tok,
				lexer::Token::OPEN_BLOCK,
				ctx,
				"Code Fragment Expected open block '{' or '@' for flags",
				result);
		}
		
		nextToken(ctx, block);
		
		tok = ctx.getCurrentToken();
		
		if (tok.type == lexer::Token::KEYWORD && tok.text == language::Keywords[language::KEYWORD_NAMESPACE]) {

			auto namespaceBlk = namespaceBlock(ctx, block);
			
			while (namespaceBlk != nullptr) {
				block->namespaces.push_back(namespaceBlk);

				

				nextToken(ctx, namespaceBlk);
				
				if (!ctx.hasTokensLeft()) {
					break;
				}

				auto& tok = ctx.getCurrentToken();
				if (tok.type == lexer::Token::CLOSE_BLOCK) {
					break;
				}

				namespaceBlk = namespaceBlock(ctx, block);
			}

			if (ctx.hasTokensLeft()) {
				StatementsBlock* res = statements(ctx, block);
				if (res) {
					block->statements = res;
				}
			}
		}
		else {
			StatementsBlock* res =  statements(ctx, block);
			if (res) {
				block->statements = res;
			}
		}
		


		return result;
	}

	language::ParseNode* moduleBlock(const lexer::FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, MODULE_BLOCK, ctx, "Parsing module, invalid state exiting parse", parent);

		verifyTokenOrFail(ctx.getCurrentToken(),
			lexer::Token::KEYWORD,
			language::Keywords[language::KEYWORD_MODULE],
			ctx,
			"Parsing module, expected module keyword",
			result);

		nextToken(ctx, parent);

		auto& moduleIdToken = ctx.getCurrentToken();

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::IDENTIFIER,
			ctx,
			"Parsing module, expected module name ",
			result);

		nextToken(ctx, parent);

		auto& tok = ctx.getCurrentToken();
		lexer::Token version;
		if (tok.type == lexer::Token::COMMA) {
			//version present
			nextToken(ctx, parent);
			version = ctx.getCurrentToken();

			verifyTokenTypeOrFail(version,
				lexer::Token::VERSION_LITERAL,
				ctx,
				"Parsing module, expected version",
				result);

			nextToken(ctx, parent);
		}
		
		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			lexer::Token::OPEN_BLOCK,
			ctx,
			"Parsing module, expected open brace '{'",
			result);
		

		

		verifyTokenTypeOrFail(ctx.lastToken(),
			lexer::Token::CLOSE_BLOCK,
			ctx,
			"module expected close block '}'",
			result);



		ModuleBlock* module = new ModuleBlock();
		module->name = moduleIdToken.text.str();
		module->version = version.text.str();
		result = module;

		CodeFragmentBlock* codeFrag = codeFragmentBlock(ctx, module);
		if (nullptr == codeFrag) {
			error(ctx.getCurrentToken(), ctx, "Parsing module, missing or invalid code fragment", result);
			return nullptr;
		}

		module->setCodeFragment(codeFrag);


		return result;
	}

	language::ParseNode* programBlock() {
		language::ParseNode* result = nullptr;

		return result;
	}

	language::ParseNode* libraryBlock() {
		language::ParseNode* result = nullptr;

		return result;
	}

	language::Comment* comment(const lexer::FileContext& ctx, language::ParseNode* parent) {
		auto& tok = ctx.getCurrentToken();
		language::Comment* result = nullptr;

		lexer::Token::Type types[] = { lexer::Token::COMMENT,lexer::Token::COMMENT_START };
		verifyTokenTypeOrFail(tok,
			types, sizeof(types)/sizeof(types[0]),
			ctx,
			"Parsing comment, expected '#' or '#{'",
			nullptr);


		
		
		

		if (tok.type == lexer::Token::COMMENT_START) {
			if (!nextToken(ctx, parent,false)) {
				error(tok, ctx, "handling multi line comments, expected comment body");
			}
			auto& comment = ctx.getCurrentToken();

			verifyTokenTypeOrFail(comment,
				lexer::Token::COMMENT,
				ctx,
				"Parsing comment, expected module keyword",
				nullptr);

			if (!nextToken(ctx,parent,false)) {
				error(tok, ctx, "handling multi line comments, expected comment close");
			}
			auto& commentEnd = ctx.getCurrentToken();

			verifyTokenTypeOrFail(commentEnd,
				lexer::Token::COMMENT_END,
				ctx,
				"Parsing comment, expected module keyword",
				nullptr);

			result = new language::Comment();
			result->parent = parent;
			result->commentsText = comment.text.str();
			result->token = comment;
		}
		else {
			result = new language::Comment();
			result->parent = parent;
			result->commentsText = tok.text.str();
			result->token = tok;
		}
		
		return result;
	}

	void commentCheck(const lexer::FileContext& ctx, language::ParseNode* parent) {
		const lexer::Token& token = ctx.getCurrentToken();

		if (token.type == lexer::Token::COMMENT_START || token.type == lexer::Token::COMMENT) {
			auto commentNode = comment(*currentCtx, parent);
			if (nullptr != commentNode) {
				if (nullptr != parent) {
					parent->comments.push_back(commentNode);
				}
				else {
					delete commentNode;
				}
			}
			nextToken(ctx, nullptr,false);
		}
	}


	language::ParseNode* root = nullptr;


	bool start() {
		bool result = true;
		currentCtx->beginTokens();
		
		language::ParseNode* tempRoot = new language::ParseNode();
		beginToken(*currentCtx, tempRoot);

		do {
			
			if (!currentCtx->hasTokensLeft()) {
				break;
			}

			const lexer::Token& token = currentCtx->getCurrentToken();

			switch (token.type) {
				case lexer::Token::AT_SIGN: {
					auto tok = peekNext(*currentCtx);
					if (tok.type == lexer::Token::OPEN_BLOCK || tok.type == lexer::Token::OPEN_BRACKET) {
						ast.root = codeFragmentBlock(*currentCtx, nullptr);
					}
					result = ast.root != nullptr;
				}
				break;

				case lexer::Token::OPEN_BLOCK: {
					ast.root = codeFragmentBlock(*currentCtx, nullptr);
					result = ast.root != nullptr;
				}
				break;

				case lexer::Token::KEYWORD: {
					if (token.text == language::Keywords[language::KEYWORD_MODULE]) {
						ast.root = moduleBlock(*currentCtx, nullptr);
					}
					else if (token.text == language::Keywords[language::KEYWORD_PROGRAM]) {
						ast.root = programBlock();
					}
					else if (token.text == language::Keywords[language::KEYWORD_LIB]) {
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

		if (nullptr != ast.root) {
			if (!tempRoot->comments.empty()) {

				for (auto comment: tempRoot->comments) {
					language::Comment* c = new language::Comment();
					c->commentsText = comment->commentsText;
					c->parent = ast.root;
					c->token = comment->token;
					ast.root->comments.push_back(c);
				}
			}
			delete tempRoot;
		}
		else {
			ast.root = tempRoot;
			result = true;
		}

		return result;
	}

	bool parse(const lexer::Lexer& lexer) {
		clear();
		currentCtx = lexer.currentContext();
		ast.root = nullptr;	

		return start();
	}
};



} //end of parser namespace


#endif //_PARSER_H__



