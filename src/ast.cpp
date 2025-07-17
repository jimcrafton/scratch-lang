#include "AST.h"



namespace language {

	ParseNode::~ParseNode()
	{
		for (auto& c : comments) {
			delete c;
		}
	}

	void ParseNode::clear()
	{
		for (auto& c : comments) {
			delete c;
		}
		comments.clear();
	}

	void Comment::accept(language::AstVisitor& v) const
	{
		v.visitComment(*this);
	}
}
