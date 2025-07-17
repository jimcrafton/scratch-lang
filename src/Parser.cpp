#include "Parser.h"

#include "AST.h"

#include "cmd_line_options.h"



namespace parser {

	void ParserOptions::init(const utils::cmd_line_options& cmdline)
	{
		auto o = cmdline["verbose-mode"];
		if (!o.is_null()) { verboseMode = o; }

		o = cmdline["debug-mode"];
		if (!o.is_null()) { debugMode = o; }

	}

	Parser::Parser(const utils::cmd_line_options& cmdline)
	{
		options.init(cmdline);
	}




	void TupleNode::accept(language::AstVisitor& v) const
	{
		v.visitTupleNode(*this);
	}

	void NamedTupleNode::accept(language::AstVisitor& v) const
	{
		v.visitNamedTupleNode(*this);
	}

	void VariableNode::accept(language::AstVisitor& v) const
	{
		v.visitVariableNode(*this);
	}

	void ParamNode::accept(language::AstVisitor& v) const 
	{
		v.visitParameterNode(*this);
	}

	void InstanceNode::accept(language::AstVisitor& v) const
	{
		v.visitInstanceNode(*this);
	}

	void MsgInstanceNode::accept(language::AstVisitor& v) const
	{
		v.visitMsgInstanceNode(*this);
	}

	void NilNode::accept(language::AstVisitor& v) const
	{
		v.visitNilNode(*this);
	}


	void LiteralNode::accept(language::AstVisitor& v) const
	{
		v.visitLiteralNode(*this);
	}


	void ArrayLiteralNode::accept(language::AstVisitor& v) const
	{
		v.visitArrayLiteralNode(*this);
	}


	void GroupedExpression::accept(language::AstVisitor& v) const
	{
		v.visitGroupedExpression(*this);
	}

	void MessageParam::accept(language::AstVisitor& v) const
	{
		v.visitMessageParam(*this);
	}

	void Message::accept(language::AstVisitor& v) const
	{
		v.visitMessage(*this);
	}


	void ReturnExpression::accept(language::AstVisitor& v) const
	{
		v.visitReturnExpression(*this);
	}


	void SendMessage::accept(language::AstVisitor& v) const
	{
		v.visitSendMessage(*this);
	}

	void Assignment::accept(language::AstVisitor& v) const
	{
		v.visitAssignment(*this);
	}


	void StatementBlock::accept(language::AstVisitor& v) const
	{
		v.visitStatementBlock(*this);
	}

	void ScopeNode::accept(language::AstVisitor& v) const
	{
		v.visitScopeNode(*this);
	}


	void StatementsBlock::accept(language::AstVisitor& v) const
	{
		v.visitStatementsBlock(*this);
	}

	void ClassBlock::accept(language::AstVisitor& v) const
	{
		v.visitClassBlock(*this);
	}


	void RecordBlock::accept(language::AstVisitor& v) const
	{
		v.visitRecordBlock(*this);
	}

	void NamespaceBlock::accept(language::AstVisitor& v) const
	{
		v.visitNamespaceBlock(*this);
	}

	void CodeFragmentBlock::accept(language::AstVisitor& v) const
	{
		v.visitCodeFragmentBlock(*this);
	}

	void ModuleBlock::accept(language::AstVisitor& v) const
	{
		v.visitModuleBlock(*this);
	}

	void MessageDeclaration::accept(language::AstVisitor & v) const
	{
		v.visitMessageDecl(*this);
	}

	void MessageBlock::accept(language::AstVisitor& v) const
	{
		v.visitMessageBlock(*this);
	}
	

}