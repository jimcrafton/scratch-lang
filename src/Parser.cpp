#include "Parser.h"

void Comment::accept(AstVisitor& v) const
{
	v.visitComment(*this);
}


void ParseNodeWithComments::accept(AstVisitor& v) const
{
	
}


void VariableNode::accept(AstVisitor& v) const
{
	v.visitVariableNode(*this);
}


void InstanceNode::accept(AstVisitor& v) const
{
	v.visitInstanceNode(*this);
}


void NilNode::accept(AstVisitor& v) const
{
	v.visitNilNode(*this);
}


void LiteralNode::accept(AstVisitor& v) const
{
	v.visitLiteralNode(*this);
}


void ArrayLiteralNode::accept(AstVisitor& v) const
{
	v.visitArrayLiteralNode(*this);
}


void GroupedExpression::accept(AstVisitor& v) const
{
	v.visitGroupedExpression(*this);
}


void Message::accept(AstVisitor& v) const
{
	v.visitMessage(*this);
}


void ReturnExpression::accept(AstVisitor& v) const
{
	v.visitReturnExpression(*this);
}


void SendMessage::accept(AstVisitor& v) const
{
	v.visitSendMessage(*this);
}

void Assignment::accept(AstVisitor& v) const
{
	v.visitAssignment(*this);
}


void StatementBlock::accept(AstVisitor& v) const
{
	v.visitStatementBlock(*this);
}

void ScopeNode::accept(AstVisitor& v) const
{
	v.visitScopeNode(*this);
}


void StatementsBlock::accept(AstVisitor& v) const
{
	v.visitStatementsBlock(*this);
}


void MessageDefinition::accept(AstVisitor& v) const
{
	v.visitMessageDefinition(*this);
}

void ClassBlock::accept(AstVisitor& v) const
{
	v.visitClassBlock(*this);
}


void RecordBlock::accept(AstVisitor& v) const
{
	v.visitRecordBlock(*this);
}

void NamespaceBlock::accept(AstVisitor& v) const
{
	v.visitNamespaceBlock(*this);
}

void CodeFragmentBlock::accept(AstVisitor& v) const
{
	v.visitCodeFragmentBlock(*this);
}

void ModuleBlock::accept(AstVisitor& v) const
{
	v.visitModuleBlock(*this);
}

