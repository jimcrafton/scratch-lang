#include "Compiler.h"


#include "Lexer.h"
#include "Parser.h"

#include <utility>
#include <cstdlib>
#include <fstream>
#include <algorithm>

#include <iostream>

#include <chrono>
#include <thread>

#include "cmd_line_options.h"
#include "version.h"

#include "win32utils.h"
#include <filesystem>
#include <thread>




namespace compiler {
	class TypeReducer : public language::AstVisitor {
	public:
		virtual ~TypeReducer() {}
		Compiler& compiler;
		TypeReducer(Compiler& c) :compiler(c) {

		}
	};

	class ModuleCheck : public language::AstVisitor {
	public:
		virtual ~ModuleCheck() {}
		Compiler& compiler;
		ModuleCheck(Compiler& c) :compiler(c) {

		}

		virtual void visitModuleBlock(const parser::ModuleBlock& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewModule(node);
			}
		}

	};

	class Stage1 : public language::AstVisitor {
	public:
		virtual ~Stage1() {}

		Compiler& compiler;
		Stage1(Compiler& c) :compiler(c) {
				
		}

			
		virtual void visitVariableNode(const parser::VariableNode& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1Variable(node);
			}
			else {
					
			}
		}

		virtual void visitNilNode(const parser::NilNode& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1Nil(node);
			}
			else {

			}
		}

		virtual void visitLiteralNode(const parser::LiteralNode& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1Literal(node);
			}
			else {

			}
		}

		virtual void visitInstanceNode(const parser::InstanceNode& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1Instance(node);
			}
			else {
					
			}
		}
			
		virtual void visitModuleBlock(const parser::ModuleBlock& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewModule(node);
			}
		}

		virtual void visitRecordBlock(const parser::RecordBlock& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewRecord(node);
			}
			else if (state == language::AstVisitor::stateVisitComplete) {
				compiler.stage1CloseRecord(node);
			}
		}

		virtual void visitClassBlock(const parser::ClassBlock& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewClass(node);
			}
			else if (state == language::AstVisitor::stateVisitComplete) {
				compiler.stage1CloseClassBlock(node);
			}
		}

		virtual void visitMessageDecl(const parser::MessageDeclaration& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewMessageDecl(node);
			}
		}

		virtual void visitMessageBlock(const parser::MessageBlock& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewMessageBlock(node);
			}
			else if(state == language::AstVisitor::stateVisitComplete) {
				compiler.stage1CloseMessageBlock(node);
			}
		}

		virtual void visitMsgInstanceNode(const parser::MsgInstanceNode& node) {
				

			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1Instance(node);
			}
			else {

			}
		}

		virtual void visitAssignment(const parser::Assignment& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewAssignmentMessage(node);
			}
		}

		virtual void visitMessageParam(const parser::SendMessage& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
			//	compiler.stage1NewSendMessage(node);
			}
		}

		virtual void visitSendMessage(const parser::SendMessage& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewSendMessage(node);
			}
		}
	};


	void CompilerOptions::init(const utils::cmd_line_options& cmdline)
	{
		auto o = cmdline[CompilerOptions::COMP_OPT_COMPILE_ONLY];

		if (!o.is_null()) { compileOnly = o; }
			
		o = cmdline[CompilerOptions::COMP_OPT_VERBOSE_MODE];
		if (!o.is_null()) { verboseMode = o; }

		o = cmdline[CompilerOptions::COMP_OPT_DEBUG_MODE];
		if (!o.is_null()) { debugMode = o; }

		o = cmdline[CompilerOptions::COMP_OPT_PRINT_AST ];
		if (!o.is_null()) { printAST = true; }

	}


		




	void Compiler::Error::output() const 
	{
		std::cout << "Compiler error: " << message << std::endl;
		compiler.outputErrors();
	}

	Compiler::Compiler(const utils::cmd_line_options& cmdline) :
		cmdlineOpts(cmdline),
		globalEnv(new compiletime::ScopedEnvironment(*this))
	{
		name = cmdlineOpts.get_app_name();
		options.init(cmdlineOpts);
		init();
	}

	Compiler::~Compiler() 
	{
		terminate();
	}

	std::string Compiler::version()
	{
		std::string result = VERSION_STR;
		return result;
	}

	std::string Compiler::logo()
	{
		std::string result = "scratch compiler, version " + Compiler::version();
		return result;
	}


	void Compiler::outputErrors() const
	{
		if (!errorListing.empty()) {
			//for (const auto& err : errorListing) {
			for (size_t i = 0; i < errorListing.size();++i) {
				const Compiler::ErrorItem& err = errorListing[i];

				std::cout << "(" << err.errCode << ")\t" << err.message << std::endl;
				std::cout << "\t" << err.code.filename << "(" << err.code.lineNumber << ":" << err.code.colNumber << ")" << std::endl;
					
				if (nullptr != err.element) {
					auto scope = err.element->getScope();						
					std::cout << "\tscope: '" << scope->getFullName() << "'" << std::endl;
				}
			}
		}
	}

	void Compiler::build(const std::vector<std::string>& files)
	{
		if (files.empty()) {
			throw Compiler::Error(*this, "no input files");
		}

		char tmp[256] = { 0 };
			
		time_t nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto t = *localtime(&nowTime);
		strftime(tmp, sizeof(tmp)-1, "Build starting at: %I:%M:%S %p (%Y/%m/%d)", &t);
		Compiler::println(tmp);

		clearEnvironment();
			
			

		lexer::Lexer lexer;

		for (auto file : files) {
			if (!options.noLogo) {
				Compiler::println(Compiler::logo() );
			}

			if (options.verboseMode) {
				Compiler::println("Starting lex phase");
			}

				

			lexer.lex(file);
				
			if (options.verboseMode) {
				Compiler::println("Starting parse phase");
			}
			parser::Parser parser(cmdlineOpts);

			parser.parse(lexer);

			if (options.printAST) {
				Compiler::println("Printing AST");
				parser.ast.print();
			}
			else {
				if (options.verboseMode) {
					Compiler::println("Starting compiling ast");
				}
				compileAST(parser.ast);
			}
				
		}
			
		if (!options.printAST) {
			compile();

			createMainEntryPoint();


			link();

			if (executableCodePtr) {
				executableCodePtr.reset();
			}
		}

		buildEnd = std::chrono::system_clock::now();

		printBuildStats();			
	}

	void Compiler::printBuildStats()
	{
		char tmp[256] = { 0 };

		auto buildTime = buildEnd - buildStart;
		const auto hrs = std::chrono::duration_cast<std::chrono::hours>(buildTime);
		const auto mins = std::chrono::duration_cast<std::chrono::minutes>(buildTime - hrs);
		const auto secs = std::chrono::duration_cast<std::chrono::seconds>(buildTime - hrs - mins);
		const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(buildTime - hrs - mins - secs);


		snprintf(tmp, sizeof(tmp) - 1, "build took (H:m:s.ms) %d:%02d:%02d.%03d", hrs.count(), mins.count(), (int)secs.count(), (int)millis.count());
		Compiler::println(tmp);
	}

	void Compiler::compileAST(const language::AST& ast)
	{
		stage1(ast);			
	}

	void Compiler::compile()
	{
		stage2();

		if (!errorListing.empty()) {
			throw Compiler::Error(*this, "Compile errors:");
		}

		if (!executableCodePtr) {
			programInst = new compiletime::Program();			
			programInst->init("a", *this);
			executableCodePtr.reset(programInst);
			
		}


		//add runtime
		


		/*
		llvm::Constant* rtConst = nullptr;
		std::string rtClassName = "TestMe";
		std::string rtVarName = "runtime";

		llvm::StructType* rtClassType = llvm::StructType::create(*llvmCtxPtr, rtClassName);

		std::vector<llvm::Type*> classMembers;
		classMembers.push_back(llvm::Type::getInt32Ty(*llvmCtxPtr));
		rtClassType->setBody(classMembers);



		llvm::GlobalVariable* rtGlobalVar = new llvm::GlobalVariable(
			*module->llvmModulePtr,                      // Module
			rtClassType,            // Type of the global variable
			false,                   // Is constant? (false for mutable)
			llvm::GlobalValue::ExternalLinkage, // Linkage type
			nullptr,                 // Initializer (nullptr for uninitialized)
			rtVarName       // Name of the global variable
		);
		*/
		/*
		llvm::AllocaInst* stackRt = llvmBuilderPtr->CreateAlloca(
			rtClassType,         // The LLVM StructType
			nullptr,               // ArraySize (nullptr for a single instance)
			rtVarName             // Name of the stack variable
		);
		*/
		//module->llvmModulePtr->getOrInsertGlobal(rtVarName, rtConst->getType());
		
		

		stage3();
	}

	void Compiler::outputModule(compiletime::Module* module, OutputFormat outFmt, const CppString& objName)
	{
		auto outputFileName = module->getName() + "_";

		auto tmpv = module->version;
		std::replace(tmpv.begin(), tmpv.end(), '.', '_');
		outputFileName += tmpv;

		switch (outFmt) {
			case OUT_INTERMEDIATE: {
				outputFileName += ".ll";
			}break;

			case OUT_OBJECT: {
				outputFileName += ".o";
			}break;
		}

		outputModule(outFmt, *module->llvmModulePtr, outputFileName);
	}

	void Compiler::link()
	{
		Linker linker(cmdlineOpts);

		if (executableCodePtr) {
			for (auto m : executableCodePtr->modules) {
				auto module = m.second;
				std::string objName;

				outputModule(module, OUT_INTERMEDIATE, objName);
				outputModule(module, OUT_OBJECT, objName);

				Compiler::println("object: " + objName);
				if (!this->options.compileOnly) {
					linker.objs.push_back(objName);
				}

			}

			std::string mainOut;
			outputMainEntryPoint(OUT_INTERMEDIATE, mainOut);
			outputMainEntryPoint(OUT_OBJECT, mainOut);
			Compiler::println("object: " + mainOut);
			if (!this->options.compileOnly) {
				linker.objs.push_back(mainOut);
			}

		}
		else {
			throw Compiler::Error(*this, "No executable section");
		}


		if (!this->options.compileOnly) {
			Compiler::println("linking executable " + executableCodePtr->getName() + " - " + executableCodePtr->version + "\n");
			linker.outputFileName = executableCodePtr->getName();

			linker.link(*this);
		}
		else {
			Compiler::println("linking disabled");
		}
	}

		

	void Compiler::print(const std::string& msg, bool emitEndln)
	{
		std::cout << msg;
		if (emitEndln) {
			std::cout << std::endl;
		}
	}

	void Compiler::println(const std::string& msg)
	{
		Compiler::print(msg, true);
	}

	std::string Compiler::mainEntryFunction()
	{
		return "main";// "runtime_main";
	}

	void Compiler::createMainEntryPoint()
	{
			
		std::string mainFuncName = Compiler::mainEntryFunction();

		
		std::string rtinit = "Runtime_init";

		mainEntryModulePtr->getOrInsertFunction(
			rtinit,
			llvm::FunctionType::get(
				llvmBuilderPtr->getInt32Ty(),
				llvmBuilderPtr->getVoidTy(), 
				false
			)
		);
		
		llvm::Type* CharTy = llvm::Type::getInt8Ty(*llvmCtxPtr);
		llvm::Type* CharPtrTy = llvm::PointerType::get(CharTy, 0);
		llvm::Type* ArgVPtrTy = llvm::PointerType::get(CharPtrTy, 0);

		


		std::string rtMain = "Runtime_main_entry_point";

		mainEntryModulePtr->getOrInsertFunction(
			rtMain,
			llvm::FunctionType::get(
				llvmBuilderPtr->getInt32Ty(),
				llvmBuilderPtr->getVoidTy(),
				false
			)
		);

		std::string rtTerm = "Runtime_terminate";

		mainEntryModulePtr->getOrInsertFunction(
			rtTerm,
			llvm::FunctionType::get(
				llvmBuilderPtr->getVoidTy(),
				llvmBuilderPtr->getVoidTy(),
				false
			)
		);
		
		
		auto returnType = llvm::FunctionType::get(llvmBuilderPtr->getInt32Ty(), llvmBuilderPtr->getVoidTy(), false);
		auto mainFunc = mainEntryModulePtr->getFunction(mainFuncName);
		if (nullptr == mainFunc) {
			auto mainFuncProto = llvm::Function::Create(returnType,
				llvm::Function::ExternalLinkage,
				mainFuncName,
				*mainEntryModulePtr);
			verifyFunction(*mainFuncProto);


			

			

			auto block = llvm::BasicBlock::Create(*llvmCtxPtr, mainFuncName, mainFuncProto);

			llvmBuilderPtr->SetInsertPoint(block);

			mainFunc = mainFuncProto;
		}
			
		mainEntryFunc = mainFunc;

		auto rtInitFunc = mainEntryModulePtr->getFunction(rtinit);
		std::vector<llvm::Value*> NoArgs;
		llvmBuilderPtr->CreateCall(rtInitFunc, NoArgs);




		

		auto rtMainFunc = mainEntryModulePtr->getFunction(rtMain);


		auto retVal = llvmBuilderPtr->CreateCall(rtMainFunc, NoArgs);



		auto rtTermFunc = mainEntryModulePtr->getFunction(rtTerm);

		llvmBuilderPtr->CreateCall(rtTermFunc, NoArgs);


		//auto mainReturnCode = 0;
		//auto retVal = llvmBuilderPtr->getInt32(mainReturnCode);
		//auto i32Res = llvmBuilderPtr->CreateIntCast(mainRetVal, llvmBuilderPtr->getInt32Ty(), true);
		llvmBuilderPtr->CreateRet(retVal);
			

	}

	void Compiler::stage1(const language::AST& ast)
	{ 
		if (nullptr == executableCodePtr) {
			programInst = new compiletime::Program();
			programInst->init("a", *this);
			executableCodePtr.reset(programInst);

			currentModule = executableCodePtr->primaryModule();
		}

		Stage1 s1(*this);
		ast.visitAll(s1);

			
		currentModule->closeCurrentScope();

		if (options.debugMode) {
			globalEnv->debugPrint();
		}
			
	}

	void Compiler::stage2 ()
	{
		//verify var types
			


		
		std::function<void(compiletime::ScopedEnvironment& scope)> func;

		func = ([&func,this](compiletime::ScopedEnvironment& scope) -> void {
				
				std::string tab = "";
				auto p = scope.parent;
				while (p) {
					p = p->parent;
					tab += "\t";
				}
					
				for (auto revIt = scope.children.rbegin(); revIt != scope.children.rend();++revIt) {
					auto s = *revIt;
					func(*s);
				}

				//do stuff
				for (const auto& v : scope.getVariables()) {
					const auto& var = v.second;
					if (var.type.type == compiletime::TypeDescriptor::typeUnknown) {
						compiletime::Variable resolvedVar;
						if (!scope.resolveVariable(var, resolvedVar)) {
							//std::cout << "var '" << var.name << "', type '" << var.type.name << "' undefined" << std::endl;
								
								
							ErrorItem err;
							if (resolvedVar.type.getName().empty()) {
								err.message = "No type is specified for this variable ('" + var.getName() + "')";									
								err.errCode = NO_VARIABLE_TYPE_NAME;
							}
							else  {
								err.message = "No type found for this variable ('" + var.getName() + "') with a type name of '" + var.type.getName() + "'";
								err.errCode = NO_VARIABLE_TYPE_DEFINED;
							}
							err.code = var.location();
							err.element = &var;
							this->errorListing.push_back(err);
						}
						else {

							std::cout << "able to resolve var '" << resolvedVar.getName() << "', type '" << resolvedVar.type.getName() << "', " << resolvedVar.type.type << std::endl;
						}
					}
				}
			});



		if (nullptr != executableCodePtr) {
			for (auto m : executableCodePtr->modules) {
				func(*m.second->globalScope());					
			}				
		}
		else {

		}

			
			
	}

		

	void Compiler::buildFunction(const compiletime::Instance* receiver, const CppString& selector, const std::vector<const compiletime::Instance*>& params , const compiletime::ScopedEnvironment& scope)
	{
		auto fullyQualifiedSelector = selector;
		bool first = true;
		for (const auto& p : params) {
			if (!first) {
				fullyQualifiedSelector += "_";
				first = false;
			}
			fullyQualifiedSelector += p->getName();
		}

	//	scope.module->llvmModulePtr

		//llvm::Function* msgFunction = this->llvmCtxPtr
	}


	void Compiler::buildFunction(const compiletime::Instance* receiver, const compiletime::Message& msg, const compiletime::ScopedEnvironment& scope)
	{
		auto selector = msg.getName();

		std::vector<const compiletime::Instance*> params;

		for (const auto& p:msg.parameters) {
			params.push_back(p.second.param);
		}

		buildFunction(receiver, selector, params, scope);
	}

	void Compiler::buildFunctions( const compiletime::ScopedEnvironment& scope)
	{
		for (const auto& m : scope.getMsgSends()) {
			const compiletime::SendMessage& sendMsg = m.second;

			//1) reciever
			//2) message selector (name of the message)
			//3) 0 or more parameters

			const compiletime::Instance* receiver = nullptr;

			receiver = sendMsg.instance;

			buildFunction(receiver, sendMsg.msg, scope);
		}
	}

	void Compiler::stage3()
	{
			

		std::function<void(compiletime::ScopedEnvironment& scope)> func;
		func = ([&func, this](compiletime::ScopedEnvironment& scope) -> void {
			for (auto revIt = scope.children.rbegin(); revIt != scope.children.rend(); ++revIt) {
				auto s = *revIt;
				func(*s);
			}

			llvm::Value* val = nullptr;
			for (const auto& i : scope.getInstances()) {
				const compiletime::Instance& inst = i.second;
					
			}
			for (const auto& v : scope.getVariables()) {
				const compiletime::Variable& var = v.second;
				auto varName = var.getName();
				switch (var.type.type) {
					case compiletime::TypeDescriptor::typeInteger8 : {
							
						//val = llvmBuilderPtr->getInt8(0);
					}break;

					default: {

					}break;
				}
			}

			this->buildFunctions(scope);

		});

		if (nullptr != executableCodePtr) {
			for (auto m : executableCodePtr->modules) {
				m.second->resetScope();
				func(*m.second->globalScope());
			}
		}
		else {

		}
	}

	void Compiler::stage1NewModule(const parser::ModuleBlock& node)
	{
			

		auto moduleName = node.name;

		if (executableCodePtr->modules.count(moduleName) != 0) {
			auto found = executableCodePtr->modules.find(moduleName);
			currentModule = found->second;
		}
		else {
			compiletime::Module* m = new compiletime::Module(*this);
			m->setLocation(node.token.location);
			m->setName(moduleName);
			m->version = node.version;
			executableCodePtr->modules.insert(std::make_pair(m->getName(), m));

			currentModule = m;
		}
			
		currentModule->resetScope();

		if (options.debugMode) {
			std::cout << "ModuleBlock " << node.name << std::endl;
		}
	}

	void Compiler::stage1NewClass(const parser::ClassBlock& node)
	{
		compiletime::TypeDecl newType;
		newType.setName(node.name);
		newType.type = compiletime::TypeDescriptor::typeClass;
		newType.setLocation(node.token.location);
		if (!currentModule->getCurrentScope()->addType(newType)) {
			throw Compiler::Error(*this, "Type already exists");
		}

		currentModule->pushNewScope();

		currentModule->getCurrentScope()->name = node.name;
	}

	void Compiler::stage1CloseClassBlock(const parser::ClassBlock& node)
	{
		currentModule->closeCurrentScope();
	}



	void Compiler::stage1NewMessageBlock(const parser::MessageBlock& node)
	{
		currentModule->pushNewScope();
		currentModule->getCurrentScope()->name = node.msgDecl->msgIsClosure ? "_closure_" : node.msgDecl->name;
	}

	void Compiler::stage1CloseMessageBlock(const parser::MessageBlock& node)
	{			
		currentModule->closeCurrentScope();
	}

	std::string Compiler::generateTempInstanceName(const language::ParseNode* node)
	{
		std::string result;

		char tmp[256] = { 0 };
		snprintf(tmp, sizeof(tmp) - 1, "%p", node);
		result = node->getFullPath() + "_tmp_" + tmp;

		return result;
	}

	void Compiler::stage1NewSendMessage(const parser::SendMessage& node)
	{
		//std::cout << "stage1NewSendMessage " << node.message->name << std::endl;
		compiletime::SendMessage newSendMsg;
		newSendMsg.msg.setName(node.message->name);
		newSendMsg.setLocation(node.token.location);

		std::string instName = node.instance->name;
		if (instName.empty()) {				
			instName = generateTempInstanceName(node.instance);
		}
		if ( currentModule->getCurrentScope()->hasInstance(instName)) {
			newSendMsg.instance = &currentModule->getCurrentScope()->getInstance(node.instance->name);
		}
		else {
			compiletime::Instance newInst;
			newInst.setName(instName);
			newInst.setLocation(node.token.location);
			currentModule->getCurrentScope()->addInstance(newInst);
			newSendMsg.instance = &currentModule->getCurrentScope()->getInstance(instName);
		}
			
		for (auto p : node.message->parameters) {

		}
		currentModule->getCurrentScope()->addSendMessage(newSendMsg);
	}
		
	void Compiler::stage1NewAssignmentMessage(const parser::Assignment& node)
	{
		compiletime::SendMessage newSendMsg;
		newSendMsg.msg.setName(node.message->name);
		newSendMsg.setLocation(node.token.location);
		std::string instName = node.instance->name;
		if (instName.empty()) {
			instName = generateTempInstanceName(node.instance);
		}
		if (currentModule->getCurrentScope()->hasInstance(instName)) {
			newSendMsg.instance = &currentModule->getCurrentScope()->getInstance(node.instance->name);
		}

		if (node.message->parameters.size() != 1) {
			throw Compiler::Error(*this,"assignment message found, but has more than 1 parameter");
		}

		currentModule->getCurrentScope()->addSendMessage(newSendMsg);
	}

	void Compiler::stage1NewMessageDecl(const parser::MessageDeclaration& node)
	{
		if (options.debugMode) {
			std::cout << "create message '" << node.getFullPath() << "', params:" << std::endl;

			for (auto p : node.params) {
				std::cout << "\t" << p->name << ", type: " << p->type << std::endl;
			}
			if (node.returnType) {
				std::cout << "\treturn type:" << std::endl;
				for (auto f : node.returnType->fields) {
					std::cout << "\t\t" << f << std::endl;
				}
			}
			else {
				std::cout << "\tno return type" << std::endl;
			}
		}



		for (const auto& p : node.params) {
			compiletime::Variable newVar;
			newVar.setName(p->name);
			newVar.setLocation(p->token.location);
			if (!p->type.empty()) {
				newVar.type = currentModule->getCurrentScope()->getType(p->type, true);
			}
			if (compiletime::TypeDescriptor::typeUnknown == newVar.type.type) {
				newVar.type.setName(p->type);
			}
			if (!currentModule->getCurrentScope()->addVariable(newVar)) {
				throw Compiler::Error(*this, "Variable already exists");
			}
		}
	}

	void Compiler::stage1NewRecord(const parser::RecordBlock& node)
	{
		compiletime::TypeDecl newType;
		newType.setName(node.name);
		newType.type = compiletime::TypeDescriptor::typeClass;
		newType.setLocation(node.token.location);

		
		if (!currentModule->getCurrentScope()->addType(newType)) {
			throw Compiler::Error(*this, "Type already exists");
		}
	}

	void Compiler::stage1CloseRecord(const parser::RecordBlock & node)
	{

	}

	void Compiler::stage1Variable(const parser::VariableNode& node)
	{
		if (options.debugMode) {
			std::cout << "variable: {" << node.getFullPath() << "}:" << node.name << std::endl;
		}
			

		compiletime::Variable newVar;
		newVar.setName( node.name);
		newVar.setLocation(node.token.location);
		if (!node.type.empty()) {

			if (!currentModule->getCurrentScope()->hasType(node.type, true)) {
				printf("ERROR! No type (%s) found!\n", node.type.c_str());
			}

			newVar.type = currentModule->getCurrentScope()->getType(node.type, true);
		}
		if (compiletime::TypeDescriptor::typeUnknown == newVar.type.type) {
			newVar.type.setName(node.type);
		}
		
		if (!currentModule->getCurrentScope()->addVariable(newVar)) {
			throw Compiler::Error(*this, "Variable already exists");
		}
	}

	void Compiler::stage1Literal(const parser::LiteralNode& node)
	{
		if (options.debugMode) {
			std::cout << "literal: " << node.getFullPath() << ": '" << node.val << "', " << node.type << std::endl;
		}

		switch (node.type) {
			case parser::LiteralNode::STRING_LITERAL: {
				datatypes::string s;
				s.assign(node.val);
				std::string name = generateTempInstanceName(&node);
				currentModule->getCurrentScope()->addStringLiteral(name, s);
					
				compiletime::Instance newInst;
				newInst.setName(name);
				newInst.type = currentModule->getCurrentScope()->getType("string", true);
				newInst.setLocation(node.token.location);
					
				newInst.value.stringPtr = &currentModule->getCurrentScope()->getStringLiteral(name);
				currentModule->getCurrentScope()->addInstance(newInst);

			}break;

			case parser::LiteralNode::INTEGER_LITERAL: {
				compiletime::Primitive p;
				p.type = globalEnv->getType("int32");
				p.data.int32V = std::stoi(node.val);


				std::string name = generateTempInstanceName(&node);
				currentModule->getCurrentScope()->addPrimLiteral(name, p);

				compiletime::Instance newInst;
				newInst.setName( name);
				newInst.type = p.type;
				newInst.setLocation(node.token.location);
					
				newInst.value.primitivePtr = &currentModule->getCurrentScope()->getPrimLiteral(name);
				currentModule->getCurrentScope()->addInstance(newInst);

			}break;
		}		
	}

	void Compiler::stage1Nil(const parser::NilNode& node)
	{
		if (options.debugMode) {
			std::cout << "nil: " << node.getFullPath() << std::endl;
		}
	}

	void Compiler::stage1Instance(const parser::MsgInstanceNode& node)
	{
		if (options.debugMode) {
			std::cout << "msg instance: " << node.getFullPath() << " : msg closure" << std::endl;
		}


		compiletime::Instance newInst;
		newInst.setName(node.name);
		newInst.setLocation(node.token.location);
		newInst.type = globalEnv->getType("message", false);
		currentModule->getCurrentScope()->addInstance(newInst);
	}

	void Compiler::stage1Instance(const parser::InstanceNode& node)
	{
		if (options.debugMode) {
			std::cout << "instance: " << node.getFullPath() << ":" << node.name << std::endl;
		}
		compiletime::Instance newInst;
		newInst.setName(node.name);
		newInst.setLocation(node.token.location);
		bool newVarNeeded = true;
		if (!node.type.empty()) {
			newInst.type = currentModule->getCurrentScope()->getType(node.type, true);
		}
		else {
			auto v = currentModule->getCurrentScope()->getVariable(node.name,true);
			if (!v.empty()) {
				newInst.type = v.type;
				newVarNeeded = false;
			}
		}
		if (compiletime::TypeDescriptor::typeUnknown == newInst.type.type) {
			newInst.type.setName(node.type);
		}
			
		currentModule->getCurrentScope()->addInstance(newInst);

		if (!currentModule->getCurrentScope()->hasVariable(newInst.getName()) && newVarNeeded) {
			compiletime::Variable newVar;
			newVar.setName(newInst.getName());
			newVar.type = newInst.type;
			newVar.setLocation(node.token.location);

			
			if (!currentModule->getCurrentScope()->addVariable(newVar)) {
				throw Compiler::Error(*this, "Variable already exists");
			}
		}			
	}


	void Compiler::clearEnvironment()
	{
		buildStart = buildEnd = std::chrono::system_clock::now();

		globalEnv->clear();
		classes.clear();
		errorListing.clear();

		programInst = nullptr;
		libInst = nullptr;
		executableCodePtr.reset();
	}

	void Compiler::init()
	{

		llvmCtxPtr = std::make_unique<llvm::LLVMContext>();
			
		llvmBuilderPtr = std::make_unique<llvm::IRBuilder<>>(*llvmCtxPtr);


		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();

		auto triple = llvm::sys::getDefaultTargetTriple();
		//triple = "x86_64-pc-windows-msvc";

		llvmTargetTriple = llvm::Triple(triple);
			

		std::string errString;
		llvmTarget = llvm::TargetRegistry::lookupTarget(triple, errString);


			

		auto cpu = llvm::sys::getHostCPUName();
		llvm::SubtargetFeatures Features;
		auto HostFeatures = llvm::sys::getHostCPUFeatures();
		if (!HostFeatures.empty()) {
			for (auto& F : HostFeatures) {
				Features.AddFeature(F.first(), F.second);
			}
		}
		auto features = Features.getString();

		llvm::TargetOptions opt;
		auto targetMachine = llvmTarget->createTargetMachine(
			llvmTargetTriple.str(), cpu, features, opt, llvm::Reloc::PIC_);
		llvmTargetMachinePtr.reset(targetMachine);


		mainEntryModulePtr = std::make_unique<llvm::Module>("mainEntryModule",  * llvmCtxPtr);
		mainEntryModulePtr->setTargetTriple(llvmTargetTriple.str());


		compiletime::Compiletime::init();

		initBasicTypes();
	}

	
	void Compiler::initBasicTypes()
	{
		static std::map<compiletime::TypeDescriptor, std::string> globalPrims = compiletime::Compiletime::globalPrims();

		for (auto t : globalPrims) {
				
			if (!globalEnv->addType(compiletime::TypeDecl(t.second, t.first))) {
				throw Compiler::Error(*this, "Type already exists");
			}
		}
	}

	llvm::Module* Compiler::createModule(const std::string& name)
	{
		llvm::Module* result = new llvm::Module(name, *llvmCtxPtr);
		result->setTargetTriple(llvmTargetTriple.str());
		return result;
	}

	void Compiler::terminate()
	{
		delete globalEnv;
	}

	void Compiler::outputModule(OutputFormat outFmt, llvm::Module& m, const std::string& outfileName)
	{
		switch (outFmt) {
			case OUT_INTERMEDIATE: {
				std::error_code ec;
				//std::string filename = "a.scratch.ll";
				llvm::raw_fd_ostream outfile(outfileName, ec);
				m.print(outfile, nullptr);
			}
			break;

			case OUT_OBJECT: {

					
				m.setDataLayout(llvmTargetMachinePtr->createDataLayout());

				std::error_code ec;
				///std::string filename = "a.scratch.o";
				llvm::raw_fd_ostream outfile(outfileName, ec);

				if (ec) {
					throw Compiler::Error (*this, "Could not open file: " + ec.message());
				}

				llvm::legacy::PassManager pass;
				auto FileType = llvm::CodeGenFileType::ObjectFile;

				if (llvmTargetMachinePtr->addPassesToEmitFile(pass, outfile, nullptr, FileType)) {
					throw Compiler::Error(*this, "targetMachine can't emit a file of this type");
				}

				pass.run(m);
				outfile.flush();
			}
			break;

		}
	}

	void Compiler::outputMainEntryPoint(OutputFormat outFmt, std::string& outfileName)
	{
		outfileName = "";
		switch (outFmt) {
			case OUT_INTERMEDIATE: {
				outfileName = executableCodePtr->getName() + "_main_entry.ll";
			}
			break;

			case OUT_OBJECT: {
				outfileName = executableCodePtr->getName() + "_main_entry.o";
			}
			break;
		}
		outputModule(outFmt,*mainEntryModulePtr, outfileName);
	}


	void LinkerOptions::init(const utils::cmd_line_options& cmdline)
	{
		auto o = cmdline["verbose-mode"];
		if (!o.is_null()) { verboseMode = o; }

		o = cmdline["debug-mode"];
		if (!o.is_null()) { debugMode = o; }

		o = cmdline["nologo"];
		if (!o.is_null()) { noLogo = o; }
	}

	void Linker::init()
	{

	}

	void Linker::terminate()
	{

	}

	void Linker::reportSuccessfulOutput()
	{
		auto sz = std::filesystem::file_size(outputFileName);

		int sizeIdx = 0;
		double mantissa = sz;
		const char* tmpSzTypes = "BKMGTPE";

		while (mantissa >= 1024.) {
			mantissa /= 1024.;
			++sizeIdx;
		}
		auto hrval = std::ceil(mantissa * 10.) / 10.;
		char tmp[256];
		snprintf(tmp, sizeof(tmp) - 1, "%0.1f%c", hrval, tmpSzTypes[sizeIdx]);
		std::string s = tmp;
		s += ((sizeIdx == 0) ? "" : "B, ");
		snprintf(tmp, sizeof(tmp) - 1, "%zu bytes", sz);
		s += ((sizeIdx == 0) ? "" : tmp);
		Compiler::println("executable: " + outputFileName + " (size: " + s + ")");
	}

	void Linker::link(const Compiler& compiler)
	{
		//run llvm linker
		std::string link_command = "/verbose /subsystem:CONSOLE /nodefaultlib /entry:" + Compiler::mainEntryFunction() + " ";
			


		if (outputFileName.find(".exe")==std::string::npos) {
			outputFileName += ".exe";
		}

		if (std::filesystem::exists(outputFileName)) {
			std::filesystem::remove(outputFileName);
		}

		link_command += "/out:" + outputFileName + " ";


		objs.push_back("scratch-lang-runtime/x64/Debug/scratch_runtime.obj");

		for (auto obj : objs) {
			link_command += obj + " ";
		}


			
		std::vector<std::string> win32CRTLibs = { "kernel32.lib", "user32.lib", "Shell32.lib"};// , "ucrt.lib", "msvcrt.lib", "vcruntime.lib"
	
		for (auto lib : win32CRTLibs) {
			link_command += lib + " ";
		}


		Compiler::println( "command: lld-link " + link_command );

		{
			utils::process process(
				[](const std::string& output) -> void {
					Compiler::println(output);
				}
			);

			process.createProcess("lld-link", link_command);
		}
			
		if (!std::filesystem::exists(outputFileName)) {
			Compiler::println("failed to create executable '" + outputFileName + "'");
		}
		else {
			reportSuccessfulOutput();
		}
	}
}
	

