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



class Assignment : public ParseNode {
public:
	ParseNode* instance;

	ParseNode* expression;
	
	virtual bool hasChildren() const {
		return true;
	}

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		children.push_back(instance);
		children.push_back(expression);
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "assignment:";

		return result;
	};
};


class StatementBlock : public ParseNode {
public:
	virtual std::string printInfo() const {
		std::string result = "statement:";

		return result;
	};
};

class ClassBlock : public ParseNode {
public:
	virtual std::string printInfo() const {
		std::string result = "class:";

		return result;
	};
};

class Comment : public ParseNode {
public:
	std::string comments;

	virtual std::string printInfo() const {
		std::string result = "comment:";

		return result;
	};
};



class StatementsBlock : public ParseNode {
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
		children.insert(children.end(), statements.begin(), statements.end());
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "statements";

		return result;
	};
};

class NamespaceBlock : public ParseNode {
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
		children.insert(children.end(), namespaces.begin(), namespaces.end());
		children.push_back(statements);
	}

	virtual std::string printInfo() const {
		std::string result = "namespace:" + name;

		return result;
	};
};



class CodeFragmentBlock : public ParseNode {
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

	virtual void getChildren(std::vector<ParseNode*>& children) const {
		children.insert(children.end(),namespaces.begin(), namespaces.end());
		children.push_back(statements);
	}

	virtual void clear() {
	}

	virtual std::string printInfo() const {
		std::string result = "code fragment";

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
				printNode(*child);
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




class ParseError  {	
public:
	const ParseNode* node = NULL;
	size_t line = 0;
	size_t col = 0;
	size_t offset = 0;
	std::string message;
	std::string errFragment;
	ParseErrorType errCode = UNKNOWN_ERR;

	ParseError(){}
	

	void output() const {		
		std::string errorText = errFragment;
		errorText += "\n\n";
		std::string spacer;
		if (col > 2) {
			spacer.append(col - 2, '-');
		}
		errorText += spacer + "^" + "\n";
		std::cout << errorText << "Error: " << message << " at line : " << line << ", " << col << std::endl;
	}
};


class Parser {
public:
		
	enum State {
		NONE = 0,
		CODE_FRAGMENT_BLOCK,
		NAMESPACE_BLOCK,
		STATEMENTS_BLOCK,
		ASSIGNMENT,
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
		ParseError parseError;
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
		ParseError parseError;
		parseError.node = nullptr;
		parseError.message = errMsg;
		parseError.col = token.colNumber;
		parseError.line = token.lineNumber;
		parseError.offset = token.offset;

		parseError.errFragment = ctx.getLine(parseError.offset);
		throw parseError;
	}

	void verifyTokenTypeOrFail(const Token& t, Token::Type type, const FileContext& ctx, const std::string& errMsg, ParseNode* node)
	{
		if (t.type != type) {
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

	void nextToken(const FileContext& ctx) {
		if (!ctx.nextToken()) {
			error(ctx.lastToken(), ctx, "Out of tokens to parse, looks like we're missing data");
		}
	}

	void prevToken(const FileContext& ctx) {
		ctx.prevToken();
	}

	ClassBlock* classBlock(const FileContext& ctx, ParseNode* parent) {
		ClassBlock* result = nullptr;

		return result;
	}

	ParseNode* expression(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;
		return result;
	}

	ParseNode* assignment(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;

		Assignment* assignmentNode = new Assignment();
		assignmentNode->parent = parent;

		result = assignmentNode;

		pushState(ASSIGNMENT);

		auto first = ctx.getCurrentToken();



		if (first.type == Token::IDENTIFIER) {
		}
		else {

		}

		popState();

		return result;
	}

	ParseNode* statement(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;

		auto first = ctx.getCurrentToken();
		
		if (first.type == Token::IDENTIFIER) {
			nextToken(ctx);
			auto tok = ctx.getCurrentToken();

			if (tok.type == Token::ASSIGMENT_OPERATOR) {
				prevToken(ctx);
				result = assignment(ctx, parent);
			}
			else {
				prevToken(ctx);
				result = expression(ctx, parent);
			}
		}
		else if (first.type == Token::OPEN_PAREN) {
			//expression
			result = expression(ctx, parent);
		}

		return result;
	}

	ParseNode* statementOrCommentOrClass(const FileContext& ctx, ParseNode* parent) {
		ParseNode* result = nullptr;
		auto first = ctx.getCurrentToken();

		if (first.type == Token::KEYWORD && first.text == "class") {
			result = classBlock(ctx, parent);
		}
		else if (first.type == Token::COMMENT) {
			Comment* comment = new Comment();
			comment->comments = first.text.str();
			comment->parent = parent;
			result = comment;
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

		nextToken(ctx);

		auto& namespaceIdToken = ctx.getCurrentToken();		

		nextToken(ctx);

		verifyTokenTypeOrFail(ctx.getCurrentToken(),
			Token::OPEN_BLOCK,
			ctx,
			"Parsing namespace, expected open brace '{'",
			result);


		nextToken(ctx);
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
			nextToken(ctx);
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
		
		pushState(STATEMENTS_BLOCK);

		auto statementBlock = statementOrCommentOrClass(*currentCtx, result);
		while (statementBlock != NULL) {
			result->statements.push_back(statementBlock);

			nextToken(*currentCtx);

			auto& tok = currentCtx->getCurrentToken();
			if (tok.type == Token::CLOSE_BLOCK) {
				break;
			}

			statementBlock = statementOrCommentOrClass(*currentCtx, result);
		}

		popState();

		return result;
	}

	ParseNode* codeFragmentBlock() {
		ParseNode* result = nullptr;

		
		verifyTokenTypeOrFail(currentCtx->getCurrentToken(),
			Token::OPEN_BLOCK,
			*currentCtx,
			"Code Fragment Expected open block '{'",
			result);

		verifyTokenTypeOrFail(currentCtx->lastToken(),
			Token::CLOSE_BLOCK,
			*currentCtx,
			"Code Fragment Expected close block '}'",
			result);



		CodeFragmentBlock* block = new CodeFragmentBlock();
		result = block;
		pushState(CODE_FRAGMENT_BLOCK);
		auto stateDepth = stateStack.size();

		nextToken(*currentCtx);
		
		auto tok = currentCtx->getCurrentToken();

		if (tok.type == Token::KEYWORD && tok.text == "namespace") {
			auto namespaceBlk = namespaceBlock(*currentCtx, block);
			while (namespaceBlk != NULL) {
				block->namespaces.push_back(namespaceBlk);

				nextToken(*currentCtx);

				auto& tok = currentCtx->getCurrentToken();
				if (tok.type == Token::CLOSE_BLOCK) {
					break;
				}

				namespaceBlk = namespaceBlock(*currentCtx, block);
			}
		}
		else {
			statements(*currentCtx, block);
		}
		

		if (stateDepth != stateStack.size() || currentState() != CODE_FRAGMENT_BLOCK) {
			error(currentCtx->getCurrentToken(), *currentCtx, "Parsing code fragment, invalid state exiting parse", result);
			return nullptr;
		}

		popState();

		return result;
	}

	ParseNode* moduleBlock() {
		ParseNode* result = nullptr;

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

	bool start() {
		bool result = true;
		currentCtx->beginTokens();

		const Token& token = currentCtx->getCurrentToken();

		switch (token.type) {
			case Token::OPEN_BLOCK: {
				ast.root = codeFragmentBlock();
				result = ast.root != nullptr;
			}
			break;

			case Token::KEYWORD: {
				if (token.text == "module") {
					ast.root = moduleBlock();
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



