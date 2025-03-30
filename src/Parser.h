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








class Comment : public language::ParseNode {
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

	virtual void accept(AstVisitor& v) const;
};


class ParseNodeWithComments : public language::ParseNode {
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

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		children.insert(children.end(), comments.begin(), comments.end());
	}
	virtual void accept(AstVisitor& v) const;
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

	virtual void accept(AstVisitor& v) const;
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

	virtual void accept(AstVisitor& v) const;
};


class NilNode : public ParseNodeWithComments {
public:
	std::string type;

	virtual ~NilNode() {
	}



	virtual std::string printInfo() const {
		std::string result = "nil ";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
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

	virtual void accept(AstVisitor& v) const;
};

class ArrayLiteralNode : public ParseNodeWithComments {
public:
	
	std::vector<language::ParseNode*> elements;

	virtual ~ArrayLiteralNode() {}


	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		
		children.insert(children.end(), elements.begin(), elements.end());
	}

	virtual std::string printInfo() const {

		std::string result = "array literal";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};

class GroupedExpression : public ParseNodeWithComments {
public:

	std::vector<language::ParseNode*> expressions;

	virtual ~GroupedExpression() {

	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.insert(children.end(), expressions.begin(), expressions.end());
	}

	virtual std::string printInfo() const {
		std::string result = "group expression";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};

class Message : public ParseNodeWithComments {
public:
	std::string name;

	std::vector<language::ParseNode*> parameters;

	virtual ~Message() {
		
		for (auto p : parameters) {
			delete p;
		}
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.insert(children.end(), parameters.begin(), parameters.end());
	}

	virtual std::string printInfo() const {
		std::string result = "message: '" + name + "'";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};

class ReturnExpression : public ParseNodeWithComments {
public:
	language::ParseNode* retVal;

	virtual ~ReturnExpression() {
		delete retVal;
	}

	virtual bool hasChildren() const {
		return true;
	}
	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.push_back(retVal);
	}

	virtual std::string printInfo() const {
		std::string result = "return: ";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};

class SendMessage : public ParseNodeWithComments {
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
		ParseNodeWithComments::getChildren(children);
		children.push_back(instance);
		children.push_back(message);
	}

	virtual std::string printInfo() const {
		std::string result = "send message:";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};


class Assignment : public SendMessage {
public:
	
	virtual ~Assignment() {}


	virtual std::string printInfo() const {
		std::string result = "assignment:";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};


class StatementBlock : public ParseNodeWithComments {
public:
	virtual std::string printInfo() const {
		std::string result = "statement:";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};

class ScopeNode : public ParseNodeWithComments {
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
		ParseNodeWithComments::getChildren(children);

		children.insert(children.end(), memberVars.begin(), memberVars.end());
	}

	virtual std::string printInfo() const {
		std::string result = "scope: " + scope;

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};



class StatementsBlock : public ParseNodeWithComments {
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
		ParseNodeWithComments::getChildren(children);

		children.insert(children.end(), statements.begin(), statements.end());
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "statements";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};

class MessageDefinition : public ParseNodeWithComments {
public:
	std::string name;

	std::vector<VariableNode*> parameters;

	StatementsBlock* statements;

	virtual ~MessageDefinition() {

		for (auto p : parameters) {
			delete p;
		}

		delete statements;
	}

	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.insert(children.end(), parameters.begin(), parameters.end());

		if (nullptr != statements) {
			children.push_back(statements);
		}
	}

	virtual std::string printInfo() const {
		std::string result = "message: '" + name + "'";

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};

class ClassBlock : public ParseNodeWithComments {
public:
	std::string name;
	std::string super;

	std::vector<ScopeNode*> scopes;
	std::vector<MessageDefinition*> messages;
	

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
		ParseNodeWithComments::getChildren(children);

		children.insert(children.end(), scopes.begin(), scopes.end());

		children.insert(children.end(), messages.begin(), messages.end());
	}

	virtual std::string printInfo() const {
		std::string result = "class: " + name + ", super: " + super;

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};



class RecordBlock : public ParseNodeWithComments {
public:
	std::string name;
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
		ParseNodeWithComments::getChildren(children);

		children.insert(children.end(), memberVars.begin(), memberVars.end());
	}

	virtual std::string printInfo() const {
		std::string result = "record: " + name + ", super: " + super;

		return result;
	};

	virtual void accept(AstVisitor& v) const;
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

	virtual void getChildren(std::vector<language::ParseNode*>& children) const {
		ParseNodeWithComments::getChildren(children);
		children.insert(children.end(), namespaces.begin(), namespaces.end());
		children.push_back(statements);
	}

	virtual std::string printInfo() const {
		std::string result = "namespace:" + name;

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};


class CodeFragmentBlock : public ParseNodeWithComments {
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
		ParseNodeWithComments::getChildren(children);
	

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

	virtual void accept(AstVisitor& v) const;
};


class ModuleBlock : public ParseNodeWithComments {
public:
	std::string name;
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
		ParseNodeWithComments::getChildren(children);
		if (codeFragment != nullptr) {
			children.push_back(codeFragment);
		}
	}

	virtual std::string printInfo() const {
		std::string result = "module:" + name;

		return result;
	};

	virtual void accept(AstVisitor& v) const;
};




class AstVisitor {
public:
	virtual void visitComment(const Comment& node) {}
	virtual void visitParseNodeWithComments(const ParseNodeWithComments& node) {}
	virtual void visitVariableNode(const VariableNode& node) {}
	virtual void visitInstanceNode(const InstanceNode& node) {}
	virtual void visitNilNode(const NilNode& node) {}
	virtual void visitLiteralNode(const LiteralNode& node) {}
	virtual void visitArrayLiteralNode(const ArrayLiteralNode& node) {}
	virtual void visitGroupedExpression(const GroupedExpression& node) {}
	virtual void visitMessage(const Message& node) {}
	virtual void visitReturnExpression(const ReturnExpression& node) {}
	virtual void visitSendMessage(const SendMessage& node) {}
	virtual void visitAssignment(const Assignment& node) {}
	virtual void visitStatementBlock(const StatementBlock& node) {}
	virtual void visitScopeNode(const ScopeNode& node) {}
	virtual void visitStatementsBlock(const StatementsBlock& node) {}
	virtual void visitMessageDefinition(const MessageDefinition& node) {}
	virtual void visitClassBlock(const ClassBlock& node) {}
	virtual void visitRecordBlock(const RecordBlock& node) {}
	virtual void visitNamespaceBlock(const NamespaceBlock& node) {}
	virtual void visitCodeFragmentBlock(const CodeFragmentBlock& node) {}
	virtual void visitModuleBlock(const ModuleBlock& node) {}
};


enum ParseErrorType {
	UNKNOWN_ERR = 0,
};

class Parser {
public:
		
	class Error {
	public:
		const language::ParseNode* node = nullptr;
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
		CLASS,
		RECORD,
		ASSIGNMENT,
		EXPRESSION,
		GROUPED_EXPRESSION,
		RETURN_EXPRESSION,
		VARIABLE,
		VARIABLE_DEFINITION,
		INSTANCE,
		SEND_MESSAGE,
		MESSAGE,
		ERROR,
	};

	class ParseStateGuard {
	public:
		size_t stateDepth = 0;
		Parser& parser;
		Parser::State state;
		const FileContext& ctx;
		std::string errMsg;
		language::ParseNode** resultPtr = nullptr;
		ParseStateGuard(Parser& p, Parser::State s, const FileContext& c, const std::string& msg, language::ParseNode* r) :
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
				language::ParseNode* n = nullptr;
				if (resultPtr != nullptr) {
					n = *resultPtr;
				}

				if (parser.currentState() != Parser::ERROR) {
					parser.error(ctx.getCurrentToken(), ctx, errMsg, n);
				}
				else {
					return;
				}
			}

			parser.popState();
		}
	};

	std::deque<State> stateStack;
	language::AST ast;
	

	const FileContext* currentCtx = nullptr;
	language::CompileFlags currentNodeFlags;

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

	void addCompilerFlagsToNode(language::ParseNode& node) {
		if (!currentNodeFlags.flags.empty()) {
			node.flags = currentNodeFlags;
			currentNodeFlags.flags.clear();
		}
		
		
	}

	void error(const Token& token, const FileContext& ctx, const std::string& errMsg) {
		pushState(ERROR);

		Parser::Error parseError;
		parseError.node = nullptr;
		parseError.message = errMsg;
		parseError.col = token.colNumber;
		parseError.line = token.lineNumber;
		parseError.offset = token.offset;

		parseError.errFragment = ctx.getLine(parseError.offset);

		throw parseError;
	}

	void error(const Token& token, const FileContext& ctx, const std::string& errMsg, language::ParseNode* result) {
		delete result;
		popState();

		pushState(ERROR);

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

	void verifyTokenTypeOrFail(const Token& t, Token::Type type, const FileContext& ctx, const std::string& errMsg, language::ParseNode* node)
	{
		if (t.type != type) {
			error(t, ctx, errMsg, node);
		}
	}

	void verifyTokenTypeOrFail(const Token& t, Token::Type* types, size_t typesSize, const FileContext& ctx, const std::string& errMsg, language::ParseNode* node)
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
		const std::string& errMsg, language::ParseNode* node)
	{		
		if (t.type != type || t.text != expectedValue) {
			error(t, ctx, errMsg, node);
		}
	}

	bool beginToken(const FileContext& ctx, language::ParseNode* parent) {
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

	bool nextToken(const FileContext& ctx, language::ParseNode* parent, bool checkForComments=true) {
		
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

	ClassBlock* classBlock(const FileContext& ctx, language::ParseNode* parent) {
		ClassBlock* result = nullptr;
		
		ParseStateGuard pg(*this, CLASS, ctx, "Parsing class statement, invalid state exiting parse", result);


		verifyTokenOrFail(ctx.getCurrentToken(),
			Token::KEYWORD,
			"class",
			ctx,
			"Parsing class, expected 'class' keyword",
			result);

		result = new ClassBlock();
		result->parent = parent;

		nextToken(ctx, parent);
		auto tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			Token::IDENTIFIER,
			ctx,
			"Parsing class, expected identifier for name of class",
			result);
		
		result->name = tok.text.str();

		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		Token::Type types[] = {Token::KEYWORD, Token::OPEN_BLOCK};
		verifyTokenTypeOrFail(tok,
			types,sizeof(types)/sizeof(types[0]),
			ctx,
			"Parsing class, expected either keyword ('inherits'), or class open block '{'",
			result);

		if (tok.type == Token::KEYWORD) {
			if (tok.text == "inherits") {
				nextToken(ctx, parent);
				tok = ctx.getCurrentToken();
				verifyTokenTypeOrFail(tok,
					Token::IDENTIFIER,
					ctx,
					"Parsing class, expected identifier for super class name '}'",
					result);

				result->super = tok.text.str();
			}

			nextToken(ctx, result);
			
		}

		tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			Token::OPEN_BLOCK,
			ctx,
			"Parsing class, expected class open block '{'",
			result);


		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		//class should be in place, now build out the innards

		while (ctx.getCurrentToken().type != Token::CLOSE_BLOCK) {

			if (tok.type == Token::KEYWORD && (tok.text == "public" || tok.text == "private")) {
				ScopeNode* scope = new ScopeNode();
				scope->parent = result;
				scope->scope = tok.text.str();

				nextToken(ctx, parent);
				tok = ctx.getCurrentToken();

				verifyTokenTypeOrFail(tok,
					Token::OPEN_BLOCK,
					ctx,
					"Parsing class, expected private/public scope open block '{'",
					result);

				nextToken(ctx, result);
				tok = ctx.getCurrentToken();



				while (ctx.getCurrentToken().type != Token::CLOSE_BLOCK) {


					VariableNode* memberVar = variableDef(ctx, scope);
					memberVar->parent = scope;

					scope->memberVars.push_back(memberVar);

					nextToken(ctx, scope);

					verifyTokenTypeOrFail(ctx.getCurrentToken(),
						Token::END_OF_STATEMENT,
						ctx,
						"Parsing class, expected end of statement (';') for public/private member variable definition",
						result);

					nextToken(ctx, scope);
				}


				tok = ctx.getCurrentToken();

				verifyTokenTypeOrFail(tok,
					Token::CLOSE_BLOCK,
					ctx,
					"Parsing class, expected private/public scope close block '}'",
					result);


				result->scopes.push_back(scope);

				nextToken(ctx, result);
				tok = ctx.getCurrentToken();
			}
			else {
				//message def
				if (tok.type == Token::AT_SIGN) {
					currentNodeFlags = compilerFlags(ctx, result);
					nextToken(ctx, result);
					tok = ctx.getCurrentToken();
				}

				if (tok.type == Token::IDENTIFIER ) {
					MessageDefinition* msgDef = new MessageDefinition();
					msgDef->parent = result;
					msgDef->name = tok.text.str();

					result->messages.push_back(msgDef);

					nextToken(ctx, result);
					tok = ctx.getCurrentToken();


					if (tok.type == Token::COLON) {
						nextToken(ctx, result);
						tok = ctx.getCurrentToken();

						//add parameters
						while (tok.type != Token::OPEN_BLOCK) {
							VariableNode* param = variableDef(ctx, msgDef);
							msgDef->parameters.push_back(param);

							nextToken(ctx, result);
							tok = ctx.getCurrentToken();
							if (tok.type == Token::COMMA) {
								nextToken(ctx, result);
								tok = ctx.getCurrentToken();
							}
						}
					}

					verifyTokenTypeOrFail(tok,
						Token::OPEN_BLOCK,
						ctx,
						"Parsing class, expected message def open block '{'",
						result);

					nextToken(ctx, result);
					tok = ctx.getCurrentToken();

					if (tok.type != Token::CLOSE_BLOCK) {
						StatementsBlock* msgStatements = statements(ctx, msgDef);
						if (msgStatements) {
							msgDef->statements = msgStatements;
						}
					}

					tok = ctx.getCurrentToken();
					verifyTokenTypeOrFail(tok,
						Token::CLOSE_BLOCK,
						ctx,
						"Parsing class, expected message def close block '}'",
						result);

					nextToken(ctx, result);
					tok = ctx.getCurrentToken();
				}
				else {
					error(tok, ctx, "Parsing class, unable to determine a member variable or a message definition", result);
				}
			}
		}


		verifyTokenTypeOrFail(tok,
			Token::CLOSE_BLOCK,
			ctx,
			"Parsing class, expected class close block '}'",
			result);


		return result;
	}



	RecordBlock* recordBlock(const FileContext& ctx, language::ParseNode* parent) {
		RecordBlock* result = nullptr;

		ParseStateGuard pg(*this, RECORD, ctx, "Parsing record statement, invalid state exiting parse", result);


		verifyTokenOrFail(ctx.getCurrentToken(),
			Token::KEYWORD,
			"record",
			ctx,
			"Parsing record, expected 'record' keyword",
			result);

		result = new RecordBlock();
		result->parent = parent;

		nextToken(ctx, parent);
		auto tok = ctx.getCurrentToken();

		verifyTokenTypeOrFail(tok,
			Token::IDENTIFIER,
			ctx,
			"Parsing record, expected identifier for name of record",
			result);

		result->name = tok.text.str();

		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		Token::Type types[] = { Token::KEYWORD, Token::OPEN_BLOCK };
		verifyTokenTypeOrFail(tok,
			types, sizeof(types) / sizeof(types[0]),
			ctx,
			"Parsing record, expected either keyword ('inherits'), or class open block '{'",
			result);

		if (tok.type == Token::KEYWORD) {
			if (tok.text == "inherits") {
				nextToken(ctx, parent);
				tok = ctx.getCurrentToken();
				verifyTokenTypeOrFail(tok,
					Token::IDENTIFIER,
					ctx,
					"Parsing record, expected identifier for super class name '}'",
					result);

				result->super = tok.text.str();
			}

			nextToken(ctx, result);

		}

		tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			Token::OPEN_BLOCK,
			ctx,
			"Parsing record, expected record open block '{'",
			result);


		nextToken(ctx, result);
		tok = ctx.getCurrentToken();

		//class should be in place, now build out the innards

		while (ctx.getCurrentToken().type != Token::CLOSE_BLOCK) {
			VariableNode* memberVar = variableDef(ctx, result);
			memberVar->parent = result;

			result->memberVars.push_back(memberVar);

			nextToken(ctx, result);

			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				Token::END_OF_STATEMENT,
				ctx,
				"Parsing record, expected end of statement (';') for member variable definition",
				result);


			nextToken(ctx, result);
			tok = ctx.getCurrentToken();
		}


		verifyTokenTypeOrFail(tok,
			Token::CLOSE_BLOCK,
			ctx,
			"Parsing record, expected record close block '}'",
			result);


		return result;
	}

	

	language::ParseNode* returnExpression(const FileContext& ctx, language::ParseNode* parent) {
		ReturnExpression* result = nullptr;

		ParseStateGuard pg(*this, RETURN_EXPRESSION, ctx, "Parsing return statement, invalid state exiting parse", result);

		result = new ReturnExpression();
		result->parent = parent;
		nextToken(ctx, result);
		result->retVal = expression(ctx, result);

		return result;
	}

	VariableNode* variableDef(const FileContext& ctx, language::ParseNode* parent) {
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


	language::ParseNode* variable(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, VARIABLE, ctx, "Parsing variable, invalid state exiting parse", result);
		Token::Type types[] = { Token::IDENTIFIER, Token::KEYWORD };

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			types, sizeof(types)/sizeof(types[0]),
			ctx,
			"Parsing variable, expected identifier or nil",
			result);

		auto tok = ctx.getCurrentToken();

		Token::Type types2[] = { Token::IDENTIFIER, Token::COLON, Token::IDENTIFIER};
		
		if (lookAhead(types, sizeof(types2) / sizeof(types2[0]), ctx)) {
			result = variableDef(ctx, parent);
		}
		else if (tok.type == Token::KEYWORD ) {
			if (tok.text == "nil") {
				NilNode* nilInst = new NilNode();
				nilInst->parent = parent;
				result = nilInst;
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
		}

		return result;
	}


	language::ParseNode* groupedExpression(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, GROUPED_EXPRESSION, ctx, "Parsing group expression, invalid state exiting parse", result);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::OPEN_PAREN,
			ctx,
			"Parsing grouped expression, expected closing paren ')'",
			result);

		nextToken(ctx, parent);

		GroupedExpression* grpExpr = new GroupedExpression();

		grpExpr->parent = parent;

		language::ParseNode* expr = expression(ctx, grpExpr);

		grpExpr->expressions.push_back(expr);

		result = grpExpr; 

		nextToken(ctx, parent);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::CLOSE_PAREN,
			ctx,
			"Parsing grouped expression, expected closing paren ')'",
			result);

		return result;
	}

	language::ParseNode* expression(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, EXPRESSION, ctx, "Parsing expression, invalid state exiting parse", result);


		auto first = ctx.getCurrentToken();
		if (first.type == Token::KEYWORD) {
			if (first.text == "return") {
				result = returnExpression(ctx, parent);
			}
		}
		else if (first.type == Token::OPEN_PAREN) {
			//should be expression!
			result = groupedExpression(ctx, parent);

			
		}
		else if (first.isLiteral()) {
			//send message

			result = sendMessage(ctx, parent);
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


	language::ParseNode* instance(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, INSTANCE, ctx, "Parsing instance, invalid state exiting parse", result);

		auto tok = ctx.getCurrentToken();

		if (tok.isLiteral()) {
			language::ParseNode* varLit = varLiteral(ctx, parent);		
			result = varLit;
		}
		else if (tok.type == Token::IDENTIFIER && peekNext(ctx).type == Token::DOT) {
			std::string varName =  tok.text.str();
			nextToken(ctx, parent);//should be DOT
			nextToken(ctx, parent);//next component...
			tok = ctx.getCurrentToken();
			while (tok.type == Token::IDENTIFIER && peekNext(ctx).type == Token::DOT) {
				varName += "." + tok.text.str();
				nextToken(ctx, parent);//should be DOT
				nextToken(ctx, parent);//next component...
				tok = ctx.getCurrentToken();
			}
			if (tok.type == Token::IDENTIFIER && peekNext(ctx).type != Token::DOT) {
				varName += "." + tok.text.str();
			}

			InstanceNode* instance = new InstanceNode();
			instance->parent = parent;
			instance->name = varName;
			result = instance;
		}
		else if (tok.type == Token::IDENTIFIER) {
			InstanceNode* instance = new InstanceNode();
			instance->parent = parent;
			instance->name = tok.text.str();
			result = instance;
		}
		

		return result;
	}

	language::ParseNode* sendMessage(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		ParseStateGuard pg(*this, SEND_MESSAGE, ctx, "Parsing send message, invalid state exiting parse", result);

		SendMessage* sendMsg = new SendMessage();
		sendMsg->parent = parent;
		result = sendMsg;

		auto tok = ctx.getCurrentToken();

		sendMsg->instance = instance(ctx, sendMsg);
		

		nextToken(ctx, sendMsg);

		auto msg = message(ctx, sendMsg);
		sendMsg->message = msg;
		
		result = sendMsg;

		return result;
	}

	language::ParseNode* assignment(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;
		
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
			language::ParseNode* var = variable(ctx, assignmentNode);
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


	language::ParseNode* varLiteral(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		auto tok = ctx.getCurrentToken();
		switch (tok.type) {
			case Token::OPEN_BRACKET: {
				bool arrayClosed = false;
				ArrayLiteralNode* arrayLit = new ArrayLiteralNode();
				arrayLit->parent = parent;

				while (nextToken(ctx,parent) && !arrayClosed) {
					tok = ctx.getCurrentToken();

					if (tok.isLiteral()) {
						language::ParseNode* lit = varLiteral(ctx, arrayLit);
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


	Message* message(const FileContext& ctx, language::ParseNode* parent) {
		Message* result = nullptr;

		
		pushState(MESSAGE);
		auto stateDepth = stateStack.size();

		Token::Type types []  = {Token::IDENTIFIER,
								Token::ASSIGMENT_OPERATOR,
								Token::ADDITION_OPERATOR,
								Token::MULT_OPERATOR,
								Token::SUBTRACTION_OPERATOR,
								Token::DIV_OPERATOR,
								Token::MOD_OPERATOR,
								Token::AT_SIGN };

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			types,sizeof(types)/sizeof(Token::Type),
			ctx,
			"Parsing message, expected identier",
			result);

		auto msgName = ctx.getCurrentToken();
		
		result = new Message();
		result->parent = parent;

		if (msgName.type == Token::AT_SIGN) {
			currentNodeFlags = compilerFlags(ctx, parent);
			nextToken(ctx, parent);
			msgName = ctx.getCurrentToken();
			addCompilerFlagsToNode(*result);
		}
		
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
			else {
				error(ctx.getCurrentToken(), ctx, "Parsing message, expecting params or end of statement", result);
			}
		}
		else if (msgName.type == Token::ASSIGMENT_OPERATOR || msgName.isMathOperator()) {
			switch (tok.type) {
				case Token::IDENTIFIER: case Token::KEYWORD: {
					language::ParseNode* var = variable(ctx, result);
					result->parameters.push_back(var);
				} break;

				case Token::OPEN_PAREN: {
					language::ParseNode* expr = expression(ctx, result);
					result->parameters.push_back(expr);
				} break;

				case Token::OPEN_BRACKET: case Token::INTEGER_LITERAL: 
				case Token::BOOLEAN_LITERAL:
				case Token::BINARY_LITERAL: case Token::DECIMAL_LITERAL:
				case Token::HEXADECIMAL_LITERAL : case Token::STRING_LITERAL : {
					language::ParseNode* literal = varLiteral(ctx, result);
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

	language::ParseNode* statement(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;

		auto first = ctx.getCurrentToken();
		
		currentNodeFlags.flags.clear();

		if (first.type == Token::AT_SIGN) {

			currentNodeFlags = compilerFlags(ctx, parent);
			nextToken(ctx, parent);

			first = ctx.getCurrentToken();
		}

		if (first.type == Token::IDENTIFIER) {
			nextToken(ctx, parent);
			auto tok = ctx.getCurrentToken();

			if (tok.type == Token::ASSIGMENT_OPERATOR) {
				prevToken(ctx);
				result = assignment(ctx, parent);
				Assignment* assignmentNode = dynamic_cast<Assignment*>(result);
				addCompilerFlagsToNode(*assignmentNode->instance);
			}
			else if (tok.type == Token::COLON) {
				//could be a var definition (i.e. foo:int8) 
				//or could be assignment ( foo:int8 := 99)
				//check for tyerminating ';' char
				Token::Type types[] = {Token::COLON, Token::IDENTIFIER, Token::END_OF_STATEMENT};
				if (lookAhead(types, sizeof(types)/sizeof(types[0]), ctx)) {
					prevToken(ctx);
					result = variableDef(ctx, parent);
					addCompilerFlagsToNode(*result);
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
		else if (first.type == Token::KEYWORD && first.text == "return") {
			
			result = expression(ctx, parent);

			nextToken(ctx, result);
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				Token::END_OF_STATEMENT,
				ctx,
				"Parsing statement, expected end of statement: ';'",
				result);
		}
		else if (first.type == Token::OPEN_PAREN) {
			//expression
			result = groupedExpression(ctx, parent);

			nextToken(ctx, result);
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				Token::END_OF_STATEMENT,
				ctx,
				"Parsing statement, expected end of statement: ';'",
				result);
		}
		else {
			error(ctx.getCurrentToken(), ctx, "Parsing statement, invalid code", result);
		}


		currentNodeFlags.flags.clear();

		return result;
	}

	language::ParseNode* statementOrCommentOrClass(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;
		auto first = ctx.getCurrentToken();

		if (first.type == Token::KEYWORD && first.text == "class") {
			result = classBlock(ctx, parent);
		}
		else if (first.type == Token::KEYWORD && first.text == "record") {
			result = recordBlock(ctx, parent);
		}
		else {
			result = statement(ctx, parent);
		}
		

		return result;
	}

	NamespaceBlock* namespaceBlock(const FileContext& ctx, language::ParseNode* parent) {
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

	StatementsBlock* statements(const FileContext& ctx, language::ParseNode* parent)
	{
		StatementsBlock* result = new StatementsBlock();
		result->parent = parent;
		
		pushState(STATEMENTS_BLOCK);

		auto statementBlock = statementOrCommentOrClass(*currentCtx, result);
		while (statementBlock != nullptr) {
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

	language::CompileFlags compilerFlags(const FileContext& ctx, language::ParseNode* parent) {
		language::CompileFlags flags;

		ParseStateGuard pg(*this, COMPILER_FLAGS, ctx, "Parsing compiler flags, invalid state exiting parse", parent);

		
		auto tok = ctx.getCurrentToken();
		verifyTokenTypeOrFail(tok,
			Token::AT_SIGN,
			ctx,
			"Expected '@' to start flags",
			nullptr);

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
				nullptr);

			
			std::string curFlag;
			while (nextToken(ctx, parent)) {
				
				tok = ctx.getCurrentToken();
				if (tok.type == Token::IDENTIFIER) {
					curFlag = tok.text.str();
				}
				else if (tok.type == Token::COMMA) {
					flags.flags.push_back(curFlag);
					curFlag = "";
				}
				else if (tok.type == Token::CLOSE_BRACKET) {
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

	CodeFragmentBlock* codeFragmentBlock(const FileContext& ctx, language::ParseNode* parent) {
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
			currentNodeFlags = compilerFlags(ctx, block);
			addCompilerFlagsToNode(*block);
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
			while (namespaceBlk != nullptr) {
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

	language::ParseNode* moduleBlock(const FileContext& ctx, language::ParseNode* parent) {
		language::ParseNode* result = nullptr;


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

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::IDENTIFIER,
			ctx,
			"Parsing module, expected module name ",
			result);

		nextToken(ctx, parent);

		auto& tok = ctx.getCurrentToken();

		if (tok.type == Token::COMMA) {
			//version present
			nextToken(ctx, parent);
			verifyTokenTypeOrFail(ctx.getCurrentToken(),
				Token::VERSION_LITERAL,
				ctx,
				"Parsing module, expected version",
				result);

			nextToken(ctx, parent);
		}
		
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
		if (nullptr == codeFrag) {
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

	language::ParseNode* programBlock() {
		language::ParseNode* result = nullptr;

		return result;
	}

	language::ParseNode* libraryBlock() {
		language::ParseNode* result = nullptr;

		return result;
	}

	void comment(const FileContext& ctx, language::ParseNode* parent) {
		auto& tok = ctx.getCurrentToken();

		Token::Type types[] = { Token::COMMENT,Token::COMMENT_START };
		verifyTokenTypeOrFail(tok,
			types, sizeof(types)/sizeof(types[0]),
			ctx,
			"Parsing comment, expected module keyword",
			nullptr);


		
		
		Comment* commentNode = nullptr;

		if (tok.type == Token::COMMENT_START) {
			if (!nextToken(ctx, parent,false)) {
				error(tok, ctx, "handling multi line comments, expected comment body");
			}
			auto& comment = ctx.getCurrentToken();

			verifyTokenTypeOrFail(comment,
				Token::COMMENT,
				ctx,
				"Parsing comment, expected module keyword",
				nullptr);

			if (!nextToken(ctx,parent,false)) {
				error(tok, ctx, "handling multi line comments, expected comment close");
			}
			auto& commentEnd = ctx.getCurrentToken();

			verifyTokenTypeOrFail(commentEnd,
				Token::COMMENT_END,
				ctx,
				"Parsing comment, expected module keyword",
				nullptr);

			commentNode = new Comment();
			commentNode->parent = parent;
			commentNode->comments = comment.text.str();
		}
		else {
			commentNode = new Comment();
			commentNode->parent = parent;
			commentNode->comments = tok.text.str();
		}
		
		if (nullptr != commentNode) {
			ParseNodeWithComments* pnc = dynamic_cast<ParseNodeWithComments*>(parent);
			if (nullptr != pnc) {
				pnc->comments.push_back(commentNode);
			}
		}
	}

	void commentCheck(const FileContext& ctx, language::ParseNode* parent) {
		const Token& token = ctx.getCurrentToken();

		if (token.type == Token::COMMENT_START || token.type == Token::COMMENT) {
			comment(*currentCtx, parent);
			nextToken(ctx, nullptr,false);
		}
	}


	bool start() {
		bool result = true;
		currentCtx->beginTokens();

		beginToken(*currentCtx,nullptr);

		do {
			

			const Token& token = currentCtx->getCurrentToken();

			switch (token.type) {
				case Token::AT_SIGN: {
					auto tok = peekNext(*currentCtx);
					if (tok.type == Token::OPEN_BLOCK || tok.type == Token::OPEN_BRACKET) {
						ast.root = codeFragmentBlock(*currentCtx, nullptr);
					}
					result = ast.root != nullptr;
				}
				break;

				case Token::OPEN_BLOCK: {
					ast.root = codeFragmentBlock(*currentCtx, nullptr);
					result = ast.root != nullptr;
				}
				break;

				case Token::KEYWORD: {
					if (token.text == "module") {
						ast.root = moduleBlock(*currentCtx, nullptr);
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



