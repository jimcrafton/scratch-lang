#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "Token.h"

namespace parser {
	class Comment;
	class ParseNodeWithComments;
	class TupleNode;
	class NamedTupleNode;
	class VariableNode;
	class ParamNode;
	class InstanceNode;
	class NilNode;
	class LiteralNode;
	class ArrayLiteralNode;
	class GroupedExpression;
	class Message;
	class ReturnExpression;
	class SendMessage;
	class Assignment;
	class StatementBlock;
	class ScopeNode;
	class StatementsBlock;
	class ClassBlock;
	class RecordBlock;
	class NamespaceBlock;
	class CodeFragmentBlock;
	class ModuleBlock;
	class MessageDeclaration;
	class MessageBlock;
	class MessageParam;
	class MsgInstanceNode;
}



namespace language {



	class CompileFlags {
	public:

		virtual ~CompileFlags() {}


		std::vector<std::string> flags;

		virtual std::string printInfo() const {
			std::string result = "flags: ";
			for (auto f : flags) {
				result += f + " ";
			}
			return result;
		};
	};


	class AstVisitor;

	class Comment;

	class ParseNode {
	protected:

	public:

		const ParseNode* parent = nullptr;
		CompileFlags flags;
		std::string name;

		lexer::Token token;

		std::vector<Comment*> comments;


		ParseNode() {}

		virtual ~ParseNode(); 

		bool hasFlags() const {
			return !flags.flags.empty();
		}

		virtual bool hasChildren() const {
			return !comments.empty();
		}

		virtual void clear();

		virtual void getChildren(std::vector<ParseNode*>& children) const {
			children.insert(children.end(), comments.begin(), comments.end());
		}

		virtual std::string printInfo() const { return std::string(); };


		virtual void accept(AstVisitor&) const {}


		std::string getFullPath() const {
			std::string result;
			auto p = parent;
			while (nullptr != p) {
				if (!p->name.empty()) {
					result = result.empty() ? p->name : p->name + "." + result;
				}
				
				p = p->parent;
			}
			
			return result;
		}
	};


	class Comment : public ParseNode {
	public:
		std::string commentsText;
		virtual ~Comment() {}

		virtual bool hasChildren() const {
			return false;
		}
		virtual std::string printInfo() const {
			std::string result = "comment: " + commentsText;

			return result;
		};

		virtual void accept(language::AstVisitor& v) const;
	};

	

	class AstVisitor {
	public:

		enum TraversalType {
			traverseUnknown = 0,
			traverseRootFirst,
			traverseLeavesFirst,
		};

		enum State {
			stateNone=0,
			stateVisitStarted,
			stateVisitChildren,
			stateVisitComplete,
		};

		TraversalType traverseType = traverseRootFirst;
		State state = AstVisitor::stateNone;

		virtual ~AstVisitor() {}

		virtual void visitComment(const language::Comment& node) {}
		virtual void visitTupleNode(const parser::TupleNode& node) {}
		virtual void visitNamedTupleNode(const parser::NamedTupleNode& node) {}
		virtual void visitVariableNode(const parser::VariableNode& node) {}
		virtual void visitParameterNode(const parser::ParamNode& node) {}
		virtual void visitInstanceNode(const parser::InstanceNode& node) {}
		virtual void visitNilNode(const parser::NilNode& node) {}
		virtual void visitLiteralNode(const parser::LiteralNode& node) {}
		virtual void visitArrayLiteralNode(const parser::ArrayLiteralNode& node) {}
		virtual void visitGroupedExpression(const parser::GroupedExpression& node) {}
		virtual void visitMessage(const parser::Message& node) {}
		virtual void visitReturnExpression(const parser::ReturnExpression& node) {}
		virtual void visitSendMessage(const parser::SendMessage& node) {}
		virtual void visitMessageParam(const parser::MessageParam& node) {}
		virtual void visitAssignment(const parser::Assignment& node) {}
		virtual void visitStatementBlock(const parser::StatementBlock& node) {}
		virtual void visitScopeNode(const parser::ScopeNode& node) {}
		virtual void visitStatementsBlock(const parser::StatementsBlock& node) {}
		virtual void visitClassBlock(const parser::ClassBlock& node) {}
		virtual void visitRecordBlock(const parser::RecordBlock& node) {}
		virtual void visitNamespaceBlock(const parser::NamespaceBlock& node) {}
		virtual void visitCodeFragmentBlock(const parser::CodeFragmentBlock& node) {}
		virtual void visitModuleBlock(const parser::ModuleBlock& node) {}
		virtual void visitMessageDecl(const parser::MessageDeclaration& node) {}
		virtual void visitMessageBlock(const parser::MessageBlock& node) {}
		virtual void visitMsgInstanceNode(const parser::MsgInstanceNode& node) {}
		
	};



	class AST {
	public:
		ParseNode* root = nullptr;

		void clear() {
			if (root) {
				delete root;
			}
			root = nullptr;

		}
		void visitNode(AstVisitor& visitor, const ParseNode& node) const {
			visitor.state = AstVisitor::stateVisitStarted;
			node.accept(visitor);
			std::vector<ParseNode*> children;
			if (node.hasChildren()) {
				
				node.getChildren(children);
				
				for (auto child : children) {
					if (child) {
						visitNode(visitor, *child);
					}
				}
			}
			visitor.state = AstVisitor::stateVisitComplete;
			node.accept(visitor);
		}

		void leafFirstVisitNode(AstVisitor& visitor, const ParseNode& node) const {			
			if (node.hasChildren()) {
				std::vector<ParseNode*> children;
				node.getChildren(children);
				for (auto childIt = children.rbegin(); childIt != children.rend(); ++childIt) {
					auto child = *childIt;
					if (child) {
						leafFirstVisitNode(visitor, *child);
					}
				}
			}
			node.accept(visitor);
		}

		void reverseVisitAll(AstVisitor& visitor) const {
			visitAll(visitor, true);
		}
		void visitAll(AstVisitor& visitor, bool leavesFirst=false) const {
			if (root != nullptr) {	
				visitor.traverseType = leavesFirst ? AstVisitor::traverseLeavesFirst : AstVisitor::traverseRootFirst;
				if (leavesFirst) {
					leafFirstVisitNode(visitor, *root);
				}
				else {
					visitNode(visitor, *root);
				}
				
			}
		}
		
		void visitNode(const ParseNode& node, const std::function<void(const ParseNode&)> & func) const {
			func(node);

			if (node.hasChildren()) {
				std::vector<ParseNode*> children;
				node.getChildren(children);
				for (auto child : children) {
					if (child) {
						visitNode(*child, func);
					}
				}
			}
		}
		
		void printNode(const ParseNode& node) {
			bool result = node.hasChildren();

			size_t depth = 0;
			const ParseNode* parent = node.parent;
			while (parent != nullptr) {
				depth++;
				parent = parent->parent;
			}

			std::string tabSpacer = "";
			tabSpacer.assign(depth, '\t');

			std::cout << tabSpacer << " ( " << node.printInfo();// << std::endl;

			if (node.hasFlags()) {
				std::cout << " < " << node.flags.printInfo() << " > ";
			}

			std::cout << std::endl;

			std::cout << tabSpacer << " ) " << std::endl;
		}

		void print() {

			if (root != nullptr) {

				visitNode(*root,
					[this](const ParseNode& node) -> void {
						this->printNode(node);
					}
				);
			}
		}
	};



	
}
