#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

class AstVisitor;


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

	class ParseNode {
	protected:

	public:
		const ParseNode* parent = nullptr;
		CompileFlags flags;

		ParseNode() {}

		virtual ~ParseNode() {}

		bool hasFlags() const {
			return !flags.flags.empty();
		}

		virtual bool hasChildren() const {
			return false;
		}

		virtual void clear() {
		}

		virtual void getChildren(std::vector<ParseNode*>& children) const {

		}

		virtual std::string printInfo() const { return std::string(); };


		virtual void accept(AstVisitor&) const {}
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
			node.accept(visitor);
			if (node.hasChildren()) {
				std::vector<ParseNode*> children;
				node.getChildren(children);
				for (auto child : children) {
					if (child) {
						visitNode(visitor, *child);
					}
				}
			}
		}

		void visitAll( AstVisitor& visitor) const {
			if (root != nullptr) {				
				visitNode(visitor, *root);
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
