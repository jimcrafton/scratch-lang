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


#include <unordered_map>

#include "llvm/Linker/Linker.h"


namespace compiler {
	
	

	class Stage1 : public language::AstVisitor {
	public:
		virtual ~Stage1() {}

		Compiler& compiler;
		Stage1(Compiler& c) :compiler(c) {
				
		}

		virtual void visitNamespaceBlock(const parser::NamespaceBlock& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewNamespace(node);
			}
			else if (state == language::AstVisitor::stateVisitComplete) {
					compiler.stage1CloseNamespace(node);
			}
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

		virtual void visitExtendsBlock(const parser::ExtendsBlock& node) {
			if (state == language::AstVisitor::stateVisitStarted) {
				compiler.stage1NewExtendsBlock(node);
			}
			else if (state == language::AstVisitor::stateVisitComplete) {
				compiler.stage1CloseExtendsBlock(node);
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

		virtual void visitReturnExpression(const parser::ReturnExpression& node) {
			if (state == language::AstVisitor::stateVisitComplete) {
				compiler.stage1NewReturnExpr(node);
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

		o = cmdline[CompilerOptions::COMP_OPT_PRINT_CODE_OBJ_MODEL];
		if (!o.is_null()) { printCodeObjModel = true; }

	}


		

	Compiler::Error::Error(Compiler& c, const std::string& err, CompilerErrorType errCode, const compiletime::CodeElement* element)
		: compiler(c)
	{
		compiler.errorListing.push_back(Compiler::ErrorItem(err, errCode, element->location(), element));
	}

	Compiler::Error::Error(Compiler& c, const Compiler::ErrorListing& errors)
		: compiler(c)
	{
		compiler.errorListing.insert(compiler.errorListing.end(), errors.begin(), errors.end());
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


			if (options.printCodeObjModel) {
				printCodeObjModel();
			}
			else {
				createMainEntryPoint();


				link();

				if (executableCodePtr) {
					executableCodePtr.reset();
				}
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

	void Compiler::outputModule(compiletime::Module* module, OutputFormat outFmt, CppString& objName)
	{
		objName = module->getName() + "_";

		auto tmpv = module->version;
		std::replace(tmpv.begin(), tmpv.end(), '.', '_');
		objName += tmpv;

		switch (outFmt) {
			case OUT_INTERMEDIATE: {
				objName += ".ll";
			}break;

			case OUT_OBJECT: {
				objName += ".o";
			}break;
		}

		outputModule(outFmt, *module->llvmModulePtr, objName);
	}

	void Compiler::link()
	{
		Linker linker(cmdlineOpts);


		llvm::Linker llvmModuleLinker(*mainEntryModulePtr);
		
		if (nullptr == executableCodePtr) {
			throw Compiler::Error(*this, "No executable section");
		}


		
		

		for (auto m : executableCodePtr->modules) {
			const auto& module = m.second;
			std::string objName; 

			


			outputModule(module, OUT_INTERMEDIATE, objName);
			outputModule(module, OUT_OBJECT, objName);

			//auto res = llvmModuleLinker.linkInModule(std::move(module->llvmModulePtr));

			//printf("module->getName: %s, res: %d\n", module->getName().c_str(), res);


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

	
	void Compiler::createExternalFunction(llvm::Module& module, const std::string& functionName, llvm::FunctionType* funcType)
	{
		module.getOrInsertFunction(functionName, funcType);
	}

	void Compiler::createMainEntryPoint()
	{
			
		std::string mainFuncName = "main";

		auto mainEntryDecl =  executableCodePtr->getStartupMain();
		
		if (nullptr == mainEntryDecl) {
			throw Compiler::Error(*this, "createMainEntryPoint, No start up declaration found!");
		}

		auto moduleMainMethod = mainEntryDecl->getFullyQualifiedName();


		std::string rtinit = "Runtime_init";

		createExternalFunction(*mainEntryModulePtr, 
			rtinit, 
			llvm::FunctionType::get(
				llvmBuilderPtr->getInt32Ty(),
				{},
				false)
		);
		

		std::string rtMain = "Runtime_main_entry_point";

		createExternalFunction(*mainEntryModulePtr,
			rtMain,
			llvm::FunctionType::get(
				llvmBuilderPtr->getInt32Ty(),
				{},
				false)
		);

		auto argsType = getLLVMType(compiletime::TypeDescriptor::typeArray);

		llvm::FunctionType* runtime_main_func = 
			llvm::FunctionType::get(llvm::Type::getInt32Ty(*llvmCtxPtr), { argsType },false);

		createExternalFunction(*mainEntryModulePtr,
			moduleMainMethod,
			runtime_main_func
		);


		
		auto moduleMainFunction = mainEntryModulePtr->getFunction(moduleMainMethod);
		


		llvm::PointerType* FuncPtrType = runtime_main_func->getPointerTo();

		std::string rtSetMain = "Runtime_set_main_func";

		createExternalFunction(*mainEntryModulePtr,
			rtSetMain,
			llvm::FunctionType::get(
				llvmBuilderPtr->getVoidTy(),
				FuncPtrType,
				false)
		);



		std::string rtTerm = "Runtime_terminate";

		createExternalFunction(*mainEntryModulePtr,
			rtTerm,
			llvm::FunctionType::get(
				llvmBuilderPtr->getVoidTy(),
				{},
				false)
		);		

		

		auto returnType = llvm::FunctionType::get(llvmBuilderPtr->getInt32Ty(), {}, false);
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

		auto moduleMainMethodVarName = mainEntryDecl->getScope()->module->getName() + "_" +  mainEntryDecl->getName() + "_ptr";
		llvm::GlobalVariable* GlobalFuncPtr = new llvm::GlobalVariable(
			*mainEntryModulePtr,
			FuncPtrType,
			false, // Not constant
			llvm::GlobalValue::ExternalLinkage,
			moduleMainFunction, // Initializer is the function itself
			moduleMainMethodVarName
		);

		llvm::LoadInst* mainFuncPtr = llvmBuilderPtr->CreateLoad(FuncPtrType, GlobalFuncPtr, "mainFuncPtr");



		auto rtSetMainFunc = mainEntryModulePtr->getFunction(rtSetMain);
		std::vector<llvm::Value*> args = { mainFuncPtr };
		auto retVal = llvmBuilderPtr->CreateCall(rtSetMainFunc, args);
		

		auto rtMainFunc = mainEntryModulePtr->getFunction(rtMain);


		retVal = llvmBuilderPtr->CreateCall(rtMainFunc, NoArgs);



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


	void Compiler::checkTypes(compiletime::ScopedEnvironment& scope)
	{		

		for (auto revIt = scope.children.rbegin(); revIt != scope.children.rend(); ++revIt) {
			auto s = *revIt;
			checkTypes(*s);
		}

		Compiler::ErrorListing errors;

		//do stuff
		for (const auto& v : scope.getVariables()) {
			const auto& var = v.second;
			if (var.typeDecl.type == compiletime::TypeDescriptor::typeUnknown) {
				compiletime::Variable resolvedVar;
				if (!scope.resolveVariable(var, resolvedVar)) {
					//std::cout << "var '" << var.name << "', type '" << var.type.name << "' undefined" << std::endl;


					ErrorItem err;
					if (resolvedVar.typeDecl.getName().empty()) {
						err.message = "No type is specified for this variable ('" + var.getName() + "')";
						err.errCode = NO_VARIABLE_TYPE_NAME;
					}
					else {
						err.message = "No type found for this variable ('" + var.getName() + "') with a type name of '" + var.typeDecl.getName() + "'";
						err.errCode = NO_VARIABLE_TYPE_DEFINED;
					}
					err.code = var.location();
					err.element = &var;
					errors.push_back(err);
				}
				else {

					std::cout << "able to resolve var '" << resolvedVar.getName() << "', type '" << resolvedVar.typeDecl.getName() << "', " << resolvedVar.typeDecl.type << std::endl;
				}
			}
		}

		if (!errors.empty()) {
			throw Compiler::Error(*this, errors);
		}
	}

	void Compiler::stage2 ()
	{
		//verify var types

		if (nullptr == executableCodePtr) {
			throw Compiler::Error(*this, "Stage 2, No executable code block found!");
		}
		
		for (auto m : executableCodePtr->modules) {
			checkTypes(*m.second->globalScope());
		}	

		//fill in compiletime instance with class info		
		for (auto m : executableCodePtr->modules) {
			auto module = m.second;
			buildClassInfo(*module->getGlobalNamespace());
		}
		
			
	}

	CppString Compiler::getElementName(const compiletime::CodeElement& element)
	{
		CppString result;

		result = element.getScope()->module->getName();

		result += "_" + element.getFullyQualifiedName();
		

		return result;
	}

	void Compiler::buildClassInfo(compiletime::NamespaceBlock& ns)
	{
		for (auto revIt = ns.namespaces.begin(); revIt != ns.namespaces.end(); ++revIt) {
			auto& s = revIt->second;
			buildClassInfo(s);
		}


		for (auto classIt : ns.code.classDefs) {
			const auto& classDef = classIt.second;
			compiletime::Class* clazz = new compiletime::Class();
			clazz->name = classDef.getName();

			auto superClazz = compiletime::Compiletime::getClass(classDef.superClassName);

			for (auto m : classDef.messages) {
				const auto& msg = m.second;

				bool staticFound = false;
				if (msg.hasFlag("static")) {
					clazz->addClassMethod(msg.getName(), msg.getNameWithArgs(), compiletime::Class::NoOpClass);
				}
				else {
					clazz->addInstanceMethod(msg.getName(), msg.getNameWithArgs(), compiletime::Class::NoOpInstance);
				}
			}


			compiletime::Compiletime::addClass(clazz, superClazz);

		}


		for (auto extIt : ns.code.extendsDefs) {
			const auto& extension = extIt.second;
			auto clazz = compiletime::Compiletime::getClass(extension.extensionTarget);


			for (auto imp : extension.implementations) {
				const auto& msgBlk = imp.second;
				bool staticFound = false;
				if (msgBlk.msgDecl->hasFlag("static")) {
					clazz->addClassMethod(msgBlk.msgDecl->getName(), msgBlk.msgDecl->getNameWithArgs(), compiletime::Class::NoOpClass);
				}
				else {
					clazz->addInstanceMethod(msgBlk.msgDecl->getName(), msgBlk.msgDecl->getNameWithArgs(), compiletime::Class::NoOpInstance);
				}
			}
		}
	}


	compiletime::ReturnValue Compiler::objectAssignment(const compiletime::Instance* receiver, const std::vector<const compiletime::Instance*>& params, const compiletime::ScopedEnvironment& scope)
	{

		return compiletime::ReturnValue(true);
	}

	compiletime::ReturnValue Compiler::primitiveStringAssignment(const compiletime::Instance* receiver, const std::vector<const compiletime::Instance*>& params, const compiletime::ScopedEnvironment& scope)
	{
		bool globalVar = false;

		if (params.empty()) {
			throw Compiler::Error(*this, "Attempting to assign to a variable instance with no RHS value");
		}

		if (params.size() != 1) {
			throw Compiler::Error(*this, "Attempting to assign more than one variable to a variable instance");
		}

		auto rhsStrInstance = params[0];

		if (receiver->variable) {
			globalVar = receiver->variable->globalVar;
		}

		if (compiletime::TypeDescriptor::typeUnknown == rhsStrInstance->typeDecl.type) {
			throw Compiler::Error(*this, "Attempting to assign RHS instance with unknown type");
		}

		if (globalVar) {
			auto lhsGlobal = scope.module->llvmModulePtr->getNamedGlobal(receiver->getName());
			lhsGlobal->setConstant(false);

			//auto llvmVal = getLLVMValue(rhsStrInstance);

			auto newString = scope.module->llvmModulePtr->getFunction("Runtime_string_new");

			auto literalVal = llvmBuilderPtr->CreateGlobalStringPtr(rhsStrInstance->value.stringPtr->c_str(), "", 0, scope.module->llvmModulePtr.get());

			//std::vector<llvm::Value*> args = { };

			//auto strVal = llvmBuilderPtr->CreateCall(newString, args);


			lhsGlobal->setInitializer((llvm::Constant*)literalVal);
		}
		else {

		}


		return compiletime::ReturnValue(true);
	}
	

	compiletime::ReturnValue Compiler::primitiveAssignment(const compiletime::Instance* receiver, const std::vector<const compiletime::Instance*>& params, const compiletime::ScopedEnvironment& scope)
	{
		bool globalVar = false;

		if (params.empty()) {
			throw Compiler::Error(*this, "Attempting to assign to a variable instance with no RHS value");
		}

		if (params.size() != 1) {
			throw Compiler::Error(*this, "Attempting to assign more than one variable to a variable instance");
		}

		auto rhsInstance = params[0];

		if (receiver->variable) {
			globalVar = receiver->variable->globalVar;
		}

		if (compiletime::TypeDescriptor::typeUnknown == rhsInstance->typeDecl.type) {
			throw Compiler::Error(*this, "Attempting to assign RHS instance with unknown type");
		}

		if (globalVar) {
			auto lhsGlobal = scope.module->llvmModulePtr->getNamedGlobal(receiver->getName());
			lhsGlobal->setConstant(false);

			auto llvmVal = getLLVMValue(rhsInstance);

			lhsGlobal->setInitializer((llvm::Constant*)llvmVal);			
		}
		else {
			auto llvmVar = scope.getLocalVar(receiver->getName());

			if (nullptr == llvmVar) {
				throw Compiler::Error(*this, "Unable to resolve receiver, local variable '" + receiver->getName() + "'");
			}

			auto llvmVal = getLLVMValue(rhsInstance);

			llvmVariableBuilderPtr->CreateStore(llvmVal, llvmVar);
		}


		return compiletime::ReturnValue(true);
	}
	

	llvm::Constant* Compiler::getLLVMValue(const compiletime::Instance* instance)
	{
		llvm::Constant* result = nullptr;
		switch (instance->typeDecl.type) {
			case compiletime::TypeDescriptor::typeBool: {
				result = instance->value.primitivePtr->data.boolV ? llvmBuilderPtr->getTrue() : llvmBuilderPtr->getFalse();
			}break;

			case compiletime::TypeDescriptor::typeInteger8: {
				result = llvmBuilderPtr->getInt8(instance->value.primitivePtr->data.int8V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger8: {

				result = llvmBuilderPtr->getInt8(instance->value.primitivePtr->data.uint8V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger16: {

				result = llvmBuilderPtr->getInt16(instance->value.primitivePtr->data.uint16V);
			}break;

			case compiletime::TypeDescriptor::typeInteger16: {

				result = llvmBuilderPtr->getInt16(instance->value.primitivePtr->data.int16V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger32: {

				result = llvmBuilderPtr->getInt32(instance->value.primitivePtr->data.uint32V);
			}break;

			case compiletime::TypeDescriptor::typeInteger32: {

				result = llvmBuilderPtr->getInt32(instance->value.primitivePtr->data.int32V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger64: {

				result = llvmBuilderPtr->getInt64(instance->value.primitivePtr->data.uint64V);
			}break;

			case compiletime::TypeDescriptor::typeInteger64: {

				result = llvmBuilderPtr->getInt64(instance->value.primitivePtr->data.int64V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger128: {
				llvm::IntegerType* uint128Ty = llvm::IntegerType::get(*llvmCtxPtr, 128);
				llvm::APInt val(128, { instance->value.primitivePtr->data.uint128V.lo, 
					instance->value.primitivePtr->data.uint128V.hi });
				result = llvm::ConstantInt::get(uint128Ty, val);

			}break;

			case compiletime::TypeDescriptor::typeInteger128: {
				llvm::IntegerType* int128Ty = llvm::IntegerType::get(*llvmCtxPtr, 128);
				llvm::APInt val(128, { instance->value.primitivePtr->data.uint128V.lo,
					instance->value.primitivePtr->data.uint128V.hi });
				result = llvm::ConstantInt::get(int128Ty, val);
				
			}break;

			case compiletime::TypeDescriptor::typeDouble32: {
				llvm::Type* halfTy = llvm::Type::getHalfTy(*llvmCtxPtr);
				result = llvm::ConstantFP::get(halfTy, llvm::APFloat( instance->value.primitivePtr->data.floatV));

			}break;

			case compiletime::TypeDescriptor::typeDouble64: {
				llvm::Type* doubleTy = llvm::Type::getDoubleTy(*llvmCtxPtr);
				result = llvm::ConstantFP::get(doubleTy, llvm::APFloat(instance->value.primitivePtr->data.doubleV));

				//printf("val: %0.5f\n", instance->value.primitivePtr->data.doubleV);
			}break;

			case compiletime::TypeDescriptor::typeString: {
				//auto str = instance->value.stringPtr;
				

			}break;

			default: {
				result = nullptr;
			}break;
		}

		return result;
	}

/*
	void Compiler::buildAssignment(const compiletime::Instance* receiver, const compiletime::Instance* param, const compiletime::ScopedEnvironment& scope)
	{
		

		auto rhsInstance = param;


		if (receiver->typeDecl.type == rhsInstance->typeDecl.type) {

			if (isTypeInt(receiver->typeDecl.type)) {
				buildIntegerAssignment(receiver, param, scope);
				return;
			}
			else if (isTypeRealNumber(receiver->typeDecl.type)) {
				buildRealAssignment(receiver, param, scope);
				return;
			}
			else {
				switch (receiver->typeDecl.type) {
				case compiletime::TypeDescriptor::typeClass:
				case compiletime::TypeDescriptor::typeRecord: {


				} break;

				case compiletime::TypeDescriptor::typeArray: {


				} break;

				case compiletime::TypeDescriptor::typeDictionary: {


				} break;

				case compiletime::TypeDescriptor::typeString: {


				} break;
				}
			}			
		}
		else {
			
		}

		switch (receiver->typeDecl.type) {
			case compiletime::TypeDescriptor::typeBool : {

			} break;

			case compiletime::TypeDescriptor::typeDouble64: {

			} break;

			case compiletime::TypeDescriptor::typeDouble32: {

			} break;

			case compiletime::TypeDescriptor::typeInteger8: {

			} break;

			case compiletime::TypeDescriptor::typeInteger16: {

			} break;

			case compiletime::TypeDescriptor::typeInteger32: {

			} break;

			case compiletime::TypeDescriptor::typeInteger64: {

			} break;

			case compiletime::TypeDescriptor::typeInteger128: {

			} break;

			default: {

			} break;
		}
	}
	*/

	/*
	void Compiler::buildFunction(const compiletime::Instance* receiver, const compiletime::MessageSelector& selector, const std::vector<const compiletime::Instance*>& params , const compiletime::ScopedEnvironment& scope)
	{
		auto fullyQualifiedSelector = selector.name;
		bool first = true;
		for (const auto& p : params) {
			if (!first) {
				fullyQualifiedSelector += "_";
				first = false;
			}
			fullyQualifiedSelector += p->getName();
		}

		switch (selector.type) {
			case compiletime::Message::msgTypeAsignmentOperator: {
				if (params.empty()) {
					throw Compiler::Error(*this, "Attempting to assign to a variable instance with no RHS value");
				}

				if (params.size() != 1) {
					throw Compiler::Error(*this, "Attempting to assign more than one variable to a variable instance");
				}

				buildAssignment(receiver, params[0], scope);
			} break;

			default: {

			} break;
		}

	//	scope.module->llvmModulePtr

		//llvm::Function* msgFunction = this->llvmCtxPtr
	}
	*/

	void Compiler::buildSendMessage(const compiletime::Instance* receiver, const compiletime::Message& msg, const compiletime::ScopedEnvironment& scope)
	{
		std::vector<const compiletime::Instance*> params;

		for (const auto& p:msg.parameters) {
			const auto& paramInst = scope.getInstance(p.second.getName());

			params.push_back(&paramInst);
		}

		auto clazz = compiletime::Compiletime::getClass(receiver->typeDecl.getName());
		
		if (nullptr == clazz) {
			//no class found??? exception!
			throw Compiler::Error(*this, "No class for reciver for type named: " + receiver->typeDecl.getName());
		}
		else {

			try {
				compiletime::ReturnValue res = clazz->sendMessage(msg.getName(), receiver, params);
			}
			catch (compiletime::Compiletime::SelectorEmpty& e) {
				throw Compiler::Error(*this, e.message);
			}
			catch (compiletime::Compiletime::MessageNotFound& e) {
				throw Compiler::Error(*this,  e.message);
			}
		}
	}


	void Compiler::buildSendMessages( const compiletime::ScopedEnvironment& scope)
	{
		for (const auto& m : scope.getMsgSends()) {
			const compiletime::SendMessage& sendMsg = m.second;

			//1) reciever
			//2) message selector (name of the message)
			//3) 0 or more parameters

			const compiletime::Instance* receiver = nullptr;

			receiver = sendMsg.instance;

			buildSendMessage(receiver, sendMsg.msg, scope);
		}
	}

	void Compiler::initModuleExternalRuntimeFunctions(compiletime::Module& module)
	{
	
		createExternalFunction(*module.llvmModulePtr,
			"Runtime_string_new_empty",

			llvm::FunctionType::get(
				llvmBuilderPtr->getPtrTy(),
				{},
				false)
		);


		llvm::Type* CharTy = llvm::Type::getInt8Ty(*llvmCtxPtr);
		llvm::Type* CharPtrTy = llvm::PointerType::get(CharTy, 0);

		createExternalFunction(*module.llvmModulePtr,
			"Runtime_string_new",
			llvm::FunctionType::get(
				llvmBuilderPtr->getPtrTy(),
				CharPtrTy,
				false)
		);
	}

	llvm::Type* Compiler::getLLVMType(const compiletime::TypeDescriptor& type)
	{
		llvm::Type* result = nullptr;

		switch (type) {
			case compiletime::TypeDescriptor::typeBool: {
				result = llvmBuilderPtr->getInt1Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger8: {
				result = llvmBuilderPtr->getInt8Ty();
			}break;

			case compiletime::TypeDescriptor::typeUInteger8: {
				result = llvmBuilderPtr->getInt8Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger16: {
				result = llvmBuilderPtr->getInt16Ty();
			}break;

			case compiletime::TypeDescriptor::typeUInteger16: {
				result = llvmBuilderPtr->getInt16Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger32: {
				result = llvmBuilderPtr->getInt32Ty();
			}break;

			case compiletime::TypeDescriptor::typeUInteger32: {
				result = llvmBuilderPtr->getInt32Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger64: {
				result = llvmBuilderPtr->getInt64Ty();
			}break;

			case compiletime::TypeDescriptor::typeUInteger64: {
				result = llvmBuilderPtr->getInt64Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger128: {
				result = llvmBuilderPtr->getInt128Ty();
			}break;

			case compiletime::TypeDescriptor::typeUInteger128: {
				result = llvmBuilderPtr->getInt128Ty();
			}break;

			case compiletime::TypeDescriptor::typeDouble32: {
				result = llvm::Type::getHalfTy(*llvmCtxPtr);
			}break;

			case compiletime::TypeDescriptor::typeDouble64: {
				result = llvm::Type::getDoubleTy(*llvmCtxPtr);
			}break;

			case compiletime::TypeDescriptor::typeArray: {

				result = llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmCtxPtr), 0);
			}break;

			case compiletime::TypeDescriptor::typeString: {

			}break;

			case compiletime::TypeDescriptor::typeDictionary: {

			}break;

			case compiletime::TypeDescriptor::typeMessage: {

			}break;

			case compiletime::TypeDescriptor::typeClass: {

			}break;

			case compiletime::TypeDescriptor::typeRecord: {

			}break;

			default: {
				result = nullptr;
			}break;
		}

		return result;
	}

	llvm::Type* Compiler::getLLVMType(const compiletime::TypeDecl& type)
	{
		return getLLVMType(type.type);
	}

	void Compiler::buildMsgInitializerBlock(const compiletime::MessageBlock& msgBlock)
	{
		auto currentScope = msgBlock.getScope();

		for (const auto& v : currentScope->getVariables()) {
			const compiletime::Variable& var = v.second;
			auto varName = var.getFullyQualifiedName();
			auto localVar = currentScope->getLocalVar(varName);

			auto varType = getLLVMType(var.typeDecl);

			llvm::Value* initVal = nullptr;

			//set to some initial value
			switch (var.typeDecl.type) {
				case compiletime::TypeDescriptor::typeDouble64: {
					auto zeroVal = llvm::ConstantFP::get(varType, 0.0);
					initVal = zeroVal;
				}break;

				case compiletime::TypeDescriptor::typeDouble32: {
					auto zeroVal = llvm::ConstantFP::get(varType, 0.0);
					initVal = zeroVal;
				}break;

				case compiletime::TypeDescriptor::typeString:
				case compiletime::TypeDescriptor::typeDictionary:
				case compiletime::TypeDescriptor::typeMessage: 
				case compiletime::TypeDescriptor::typeClass:
				case compiletime::TypeDescriptor::typeRecord: {
					llvm::PointerType* ptrType = llvm::cast<llvm::PointerType>(localVar->getType());
					llvm::Constant* nilPtr = llvm::ConstantPointerNull::get(ptrType);
					initVal = nilPtr;
				}break;

				default: {
					//llvm::Type* intType = llvm::Type::getInt32Ty(*llvmCtxPtr);
					initVal = llvm::ConstantInt::get(varType, 0);

				}break;
			}

			if (nullptr != initVal) {
				llvmBuilderPtr->CreateStore(initVal, localVar);
			}
		}

	}

	void Compiler::buildMsgPreConditionBlock(const compiletime::MessageBlock& msgBlock)
	{

	}

	void Compiler::buildMsgPostConditionBlock(const compiletime::MessageBlock& msgBlock)
	{

	}
	
	void Compiler::buildMsgFinalizeBlock(const compiletime::MessageBlock& msgBlock)
	{

	}
	

	void Compiler::buildCode(const compiletime::MessageBlock& msgBlock)
	{
		auto staticMessage = msgBlock.msgDecl->hasFlag("static");
		
		auto msgName = msgBlock.msgDecl->getFullyQualifiedName();

		auto currentScope = msgBlock.getScope();
		
		llvm::Type* returnType = getLLVMType(msgBlock.msgDecl->returnType);

		auto msgParams = msgBlock.msgDecl->parameters;

		

		//self is an object pointer?
		
	 	const auto& classType = currentScope->getType(msgBlock.className, true);
		llvm::Type* selfType = classType.getType( llvmBuilderPtr.get() );

		 //= llvm::PointerType::get(llvm::Type::getInt8Ty(*llvmCtxPtr), 0);

		std::vector<llvm::Type*> argTypes;

		if (!staticMessage) {
			//need to put in self param?
			argTypes.push_back(selfType);
		}
		

		for (auto p : msgParams) {
			const auto& param = p.second;
			auto paramType = getLLVMType(param.typeDecl);//  param->typeDecl);
			argTypes.push_back(paramType);
		}

		auto functionType = llvm::FunctionType::get(returnType, argTypes, false);
		auto functionProto =
			llvm::Function::Create(functionType, llvm::Function::ExternalLinkage,
				msgName,
				*msgBlock.getScope()->module->llvmModulePtr);


		size_t i = 0;
		if (!staticMessage) {
			auto arg = functionProto->getArg(0);
			arg->setName("self");
			i++;
		}

		for (auto p : msgParams) {
			auto arg = functionProto->getArg(i);
			arg->setName(p.second.getName());
			i++;
		}

		verifyFunction(*functionProto);

		auto block = llvm::BasicBlock::Create(*llvmCtxPtr, msgName, functionProto);
		llvmBuilderPtr->SetInsertPoint(block);
		
		
		for (const auto& v : currentScope->getVariables()) {
			const compiletime::Variable& var = v.second;
			auto varName = var.getFullyQualifiedName();



			llvmVariableBuilderPtr->SetInsertPoint(&functionProto->getEntryBlock());

			llvm::Type* varType = getLLVMType(var.typeDecl);

			auto localVar = llvmVariableBuilderPtr->CreateAlloca(varType, 0, varName);
			//llvmVariableBuilderPtr->CreateLoad(localVar->getAllocatedType(), localVar, varName);

			currentScope->addLocalVar(varName, localVar);
		}

		//initia 
		buildMsgInitializerBlock(msgBlock);

		//initia 
		buildMsgPreConditionBlock(msgBlock);

		buildSendMessages(*currentScope);


		buildMsgPostConditionBlock(msgBlock);

		//we need to create a tuple for the return type....


		if (nullptr != msgBlock.returnVal) {
			auto retVal = getLLVMValue(msgBlock.returnVal);

			llvmBuilderPtr->CreateRet(retVal);
		}


		buildMsgFinalizeBlock(msgBlock);
	}

	void Compiler::buildCode(const compiletime::ExtendsBlock& extensionBlock)
	{
		for (const auto& impl : extensionBlock.implementations) {
			buildCode(impl.second);
		}
	}

	void Compiler::buildCode(const compiletime::ClassBlock& classBlock)
	{

	}

	void Compiler::buildCode(const compiletime::NamespaceBlock& ns, const compiletime::Module& module)
	{
		
		auto scope = ns.getScope();

		for (const auto& v : scope->getVariables()) {
			const compiletime::Variable& var = v.second;
			auto varName = var.getFullyQualifiedName();


			if (var.globalVar) {
				llvm::Type* globalVarType = var.typeDecl.getType(llvmBuilderPtr.get());
				module.llvmModulePtr->getOrInsertGlobal(varName, globalVarType);
			}
			else {
				//stack or register vars???

			}
		}


		for (const auto& childNs : ns.namespaces) {
			buildCode(childNs.second, module);
		}



		for (const auto& extension : ns.code.extendsDefs) {
			buildCode(extension.second);
		}

		for (const auto& classDef : ns.code.classDefs) {
			buildCode(classDef.second);
		}
	}

	void Compiler::buildCode(compiletime::Module& module)
	{
		buildCode(*module.getGlobalNamespace(), module);
	}

	void Compiler::stage3()
	{
		// not needed anymore because stage 2 does this but,,, 
		//just to be paranoid....
		if (nullptr == executableCodePtr) {
			throw Compiler::Error(*this, "No executable code block found!");
		}


		if (options.printCodeObjModel) {
			return;//no code to generate
		}

		int mainMethodFoundCount = executableCodePtr->findStartup();

		if (mainMethodFoundCount == 0) {
			throw Compiler::Error(*this, "No 'main' method found for any module. Unable to continue without this");
		}
		else if (mainMethodFoundCount > 1) {
			throw Compiler::Error(*this, "Multiple instances of 'main' method found in modules. Unable to determine which one to use.");
		}
		
		for (auto m : executableCodePtr->modules) {
			auto& module = *m.second;

			module.resetScope();

			initModuleExternalRuntimeFunctions(module);

			buildCode(module);
		}
		
	}

	void Compiler::stage1NewNamespace(const parser::NamespaceBlock& node)
	{
		
		auto ns = currentModule->createNewNamespace(node.name);
		ns->setLocation(node.token.location);

		ns->setScope(currentModule->pushNewScope(ns->getName(), ns));

	}

	void Compiler::stage1CloseNamespace(const parser::NamespaceBlock& node)
	{
		currentModule->closeCurrentScope();

		currentModule->popCurrentNamespace();
	}

	compiletime::Module* Compiler::createModule(const parser::ModuleBlock& node)
	{
		auto result = new compiletime::Module(*this);
		result->setName(node.name);
		result->version = node.version;
		result->setLocation(node.token.location);

		result->init(*this);

		return result;
	}

	void Compiler::stage1NewModule(const parser::ModuleBlock& node)
	{
			

		auto moduleName = node.name;

		if (executableCodePtr->modules.count(moduleName) != 0) {
			auto found = executableCodePtr->modules.find(moduleName);
			currentModule = found->second;
		}
		else {
			auto m = createModule(node);
			
			executableCodePtr->modules.insert(std::make_pair(m->getName(), m));

			currentModule = m;
		}
			
		currentModule->resetScope();


		auto scope = currentModule->globalScope();

		compiletime::TypeDecl newType;
		
		newType.setName(currentModule->getName());
		newType.type = compiletime::TypeDescriptor::typeClass;	

		scope->addType(newType);


		compiletime::Class* newClass = new compiletime::Class();
		newClass->name = newType.getName();
		compiletime::Compiletime::addClass(newClass, compiletime::Compiletime::getClass("module"));


		if (options.debugMode) {
			std::cout << "ModuleBlock " << node.name << std::endl;
		}
	}

	void Compiler::stage1NewExtendsBlock(const parser::ExtendsBlock& node)
	{
		auto globalNS = currentModule->getGlobalNamespace();

		//printf("extension %s starting\n", node.objectToExtend.c_str());

		compiletime::ExtendsBlock extensionBlk;
		extensionBlk.setName(node.name + "_" + node.objectToExtend);
		extensionBlk.extensionTarget = node.objectToExtend;
		extensionBlk.setLocation(node.token.location);

		auto res = globalNS->code.extendsDefs.insert(std::make_pair(extensionBlk.getName(), extensionBlk));
		auto& extends = res.first->second;

		extends.setScope(currentModule->pushNewScope(extensionBlk.getName(), &extends));
		currentModule->setCurrentExtendsBlock(&extends);

	}

	void Compiler::stage1CloseExtendsBlock(const parser::ExtendsBlock& node)
	{
		currentModule->setCurrentExtendsBlock(nullptr);
		currentModule->closeCurrentScope();
		//printf("extension %s closing\n", node.objectToExtend.c_str());
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


		compiletime::ClassBlock newClass;
		initCodeElement(newClass, node.name, node);

		
		auto* currentNs = currentModule->getCurrentNamespace();
		auto res = currentNs->code.classDefs.insert(std::make_pair(newClass.getName(), newClass));
		auto& cls = res.first->second;
		
		cls.setScope(currentModule->pushNewScope(cls.getName(), &cls));
		currentModule->setCurrentClassBlock(&cls);

	}

	void Compiler::stage1CloseClassBlock(const parser::ClassBlock& node)
	{
		currentModule->setCurrentClassBlock(nullptr);
		currentModule->closeCurrentScope();
	}

	void Compiler::initCodeElement(compiletime::CodeElement& element, const CppString& name, const language::ParseNode& node)
	{
		element.setNamespace(currentModule->getCurrentNamespace());
		element.setScope(currentModule->getCurrentScope());
		element.setLocation(node.token.location);
		element.setName(name);
	}

	void Compiler::stage1NewMessageBlock(const parser::MessageBlock& node)
	{
		currentModule->setCurrentMessageBlock(nullptr);
		auto scopeName = node.msgDecl->msgIsClosure ? "_closure_" : node.msgDecl->name;
		currentModule->pushNewScope(scopeName, nullptr);
		//currentModule->getCurrentScope()->name = 

		compiletime::MessageBlock newMsgBlk;
		
		initCodeElement(newMsgBlk, node.name, node);
		
		compiletime::MessageDecl* msgDecl = nullptr;

		auto extends = currentModule->getCurrentExtendsBlock();
		if (nullptr != extends) {
			auto found = extends->messages.find(newMsgBlk.getName());
			if (found != extends->messages.end()) {
				msgDecl = &found->second;
				newMsgBlk.className = extends->extensionTarget;
			}
		}
		else {
			auto classBlk = currentModule->getCurrentClassBlock();
			if (nullptr != classBlk) {
				auto found = classBlk->messages.find(newMsgBlk.getName());
				if (found != classBlk->messages.end()) {
					msgDecl = &found->second;
					newMsgBlk.className = classBlk->getName();
				}
			}
		}


		


		for (auto p : node.msgDecl->params) {
			compiletime::MessageParameter param;
			auto paramName = p->name;
			compiletime::TypeDecl instanceType;
			const compiletime::Instance* inst = nullptr;

			if (!currentModule->getCurrentScope()->hasType(p->type,true)) {
				throw Compiler::Error(*this, "Message param '" + p->name + "' Type '" + p->type + "' not found");
			}
			else {
				instanceType = currentModule->getCurrentScope()->getType(p->type);
			}

			if (!currentModule->getCurrentScope()->hasInstance(paramName)) {
				compiletime::Instance newInst;
				initCodeElement(newInst, paramName, *p);
				newInst.typeDecl = instanceType;
				
				newInst.variable = &currentModule->getCurrentScope()->parent->getVariable(paramName);

				inst = currentModule->getCurrentScope()->addInstance(newInst);
			}

			
			

			//auto res = newSendMsg.msg.parameters.insert(std::make_pair(param.getName(), param));
		//	auto& msgParam = res.first->second;
			//msgParam.setScope(currentModule->getCurrentScope());
			//msgParam.setNamespace(currentModule->getCurrentNamespace());

			//msgParam.param = &currentModule->getCurrentScope()->getInstance(msgParam.getName());

		}



		for (const auto& stmt : node.statements->statements) {
			
		}




		extends = currentModule->getCurrentExtendsBlock();
		if (nullptr != extends) {
			auto res = extends->implementations.insert(std::make_pair(newMsgBlk.getName(), newMsgBlk));
			res.first->second.setScope(currentModule->getCurrentScope());
			res.first->second.setNamespace(currentModule->getCurrentNamespace());

			res.first->second.msgDecl = msgDecl;
			currentModule->setCurrentMessageBlock(&res.first->second);
		}
		else {
			auto classBlk = currentModule->getCurrentClassBlock();
			if (nullptr != classBlk) {
				auto res = classBlk->implementations.insert(std::make_pair(newMsgBlk.getName(), newMsgBlk));
				res.first->second.setScope(currentModule->getCurrentScope());
				res.first->second.setNamespace(currentModule->getCurrentNamespace());

				res.first->second.msgDecl = msgDecl;
				currentModule->setCurrentMessageBlock(&res.first->second);
			}
			else {
				throw Compiler::Error(*this, "message '" + node.name + "' declared outside of a class or extension block");
			}
		}

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

	void Compiler::stage1NewReturnExpr(const parser::ReturnExpression& node)
	{
		if (nullptr != node.retVal) {			
			
			std::string name = generateTempInstanceName(node.retVal);

			const auto& inst = currentModule->getCurrentScope()->getInstance(name);
			
			auto msgBlk = currentModule->getCurrentMessageBlock();
			msgBlk->returnVal = &inst;
		}
		else {
			auto msgBlk = currentModule->getCurrentMessageBlock();
			msgBlk->returnVal = nullptr;
		}
	}

	void Compiler::stage1NewSendMessage(const parser::SendMessage& node)
	{
		//std::cout << "stage1NewSendMessage " << node.message->name << std::endl;
		compiletime::SendMessage newSendMsg;		

		initCodeElement(newSendMsg, node.message->name, node);
		
		newSendMsg.msg.setName(node.message->name);

		if (node.message->token.isMathOperator()) {
			newSendMsg.msg.type = compiletime::Message::msgTypeMathOperator;
		}
		else if (node.message->token.type == lexer::Token::ASSIGMENT_OPERATOR) {
			newSendMsg.msg.type = compiletime::Message::msgTypeAsignmentOperator;
		}
		else if (node.message->token.type == lexer::Token::EQUALS_SIGN) {
			newSendMsg.msg.type = compiletime::Message::msgTypeCompareEquals;
		}
		else if (node.message->name == "<" || node.message->name == ">") {
			newSendMsg.msg.type = compiletime::Message::msgTypeComparison;
		}
		else {
			newSendMsg.msg.type = compiletime::Message::msgTypeNamed;
		}
		
		

		std::string instName = node.instance->name;
		if (instName.empty()) {				
			instName = generateTempInstanceName(node.instance);
		}
		if ( currentModule->getCurrentScope()->hasInstance(instName)) {
			newSendMsg.instance = &currentModule->getCurrentScope()->getInstance(node.instance->name);
		}
		else {
			compiletime::TypeDecl instanceType;
			const compiletime::Variable* existingVar = nullptr;

			if (currentModule->getCurrentScope()->hasVariable(instName)) {
				 const auto& var = currentModule->getCurrentScope()->getVariable(instName);
				 instanceType = var.typeDecl;
				 existingVar = &var;
			}


			compiletime::Instance newInst;
			initCodeElement(newInst, instName, node);			
			newInst.typeDecl = instanceType;
			newInst.variable = existingVar;
			currentModule->getCurrentScope()->addInstance(newInst);
			newSendMsg.instance = &currentModule->getCurrentScope()->getInstance(instName);
		}
			
		for (auto p : node.message->parameters) {
			compiletime::MessageParameter param;

			std::string name = generateTempInstanceName(p->param);
			//std::cout << "stage1NewSendMessage param inst: " << name << std::endl;
			
			
			initCodeElement(param, name, *p);

			newSendMsg.msg.parameters.push_back(std::make_pair(param.getName(), param));
			//auto& res = newSendMsg.msg.parameters.back();
			//auto& msgParam =  res.second;
			//msgParam.setScope(currentModule->getCurrentScope());
			//msgParam.setNamespace(currentModule->getCurrentNamespace());

			//msgParam.param = &currentModule->getCurrentScope()->getInstance(msgParam.getName());

		}
		currentModule->getCurrentScope()->addSendMessage(newSendMsg);
	}
		
	void Compiler::stage1NewAssignmentMessage(const parser::Assignment& node)
	{
		compiletime::SendMessage newSendMsg;		

		initCodeElement(newSendMsg, node.message->name, node);

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

		compiletime::MessageDecl newMsgDecl;
		initCodeElement(newMsgDecl, node.name, node);


		for (auto f : node.flags.flags) {
			compiletime::Flag newFlag;
			newFlag.val = f.second;
			newMsgDecl.getFlags().insert( std::make_pair(f.first, newFlag) );
		}


		for (const auto& p : node.params) {
			compiletime::Variable newVar;
			
			initCodeElement(newVar, p->name, *p);

			if (!p->type.empty()) {
				newVar.typeDecl = currentModule->getCurrentScope()->getType(p->type, true);
			}
			if (compiletime::TypeDescriptor::typeUnknown == newVar.typeDecl.type) {
				newVar.typeDecl.setName(p->type);
			}
			if (!currentModule->getCurrentScope()->addVariable(newVar)) {
				throw Compiler::Error(*this, "Variable already exists", VARIABLE_ALREADY_EXISTS, &newVar);
			}
			compiletime::MessageParameter newParam;
			initCodeElement(newParam, p->name, *p);
			newParam.typeDecl = newVar.typeDecl;
			
			newMsgDecl.parameters.push_back( std::make_pair(p->name, newParam) );
		}

		if (nullptr == node.returnType) {
			
		}
		else {
			if (node.returnType->fields.empty()) {
				throw Compiler::Error(*this, "no fields for return tuple ");
			}
			newMsgDecl.returnType = currentModule->getCurrentScope()->getType(node.returnType->fields[0], true);
		}

		

		auto extends =  currentModule->getCurrentExtendsBlock();
		if (nullptr!= extends) {
			newMsgDecl.className = extends->extensionTarget;
			extends->messages.insert(std::make_pair(newMsgDecl.getName(), newMsgDecl));
		}
		else {
			auto classBlk = currentModule->getCurrentClassBlock();
			if (nullptr != classBlk) {
				newMsgDecl.className = classBlk->getName();
				classBlk->messages.insert(std::make_pair(newMsgDecl.getName(), newMsgDecl));
			}
			else {
				throw Compiler::Error(*this, "message '" + node.name + "' declared outside of a class or extension block", MESSAGE_DECL_OUTSIDE, &newMsgDecl);
			}			
		}
		
	}

	void Compiler::stage1NewRecord(const parser::RecordBlock& node)
	{
		compiletime::TypeDecl newType;
		
		newType.type = compiletime::TypeDescriptor::typeRecord;		

		initCodeElement(newType, node.name, node);
		
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

		initCodeElement(newVar, node.name, node);

		if (!node.type.empty()) {

			if (!currentModule->getCurrentScope()->hasType(node.type, true)) {
				printf("ERROR! No type (%s) found!\n", node.type.c_str());
			}

			newVar.typeDecl = currentModule->getCurrentScope()->getType(node.type, true);
		}
		if (compiletime::TypeDescriptor::typeUnknown == newVar.typeDecl.type) {
			newVar.typeDecl.setName(node.type);
		}

		const auto& scope = *currentModule->getCurrentScope();
		newVar.globalVar = nullptr == scope.parent;

		if (nullptr != scope.parent) {

			newVar.globalVar = (nullptr == scope.parent->parent) && (scope.isGlobal());
		}

		if (!newVar.globalVar) {
			//one more check
			if (nullptr != scope.scopedElement) {				
				if (scope.scopedElement->scopeConsideredGlobal()) {
					newVar.globalVar = true;
				}
			}
		}
		
		if (!currentModule->getCurrentScope()->addVariable(newVar)) {			
			throw Compiler::Error(*this, "Variable already exists", VARIABLE_ALREADY_EXISTS, &newVar);
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
				initCodeElement(newInst, name, node);
				
				newInst.typeDecl = currentModule->getCurrentScope()->getType("string", true);
				
					
				newInst.value.stringPtr = &currentModule->getCurrentScope()->getStringLiteral(name);
				currentModule->getCurrentScope()->addInstance(newInst);

			}break;

			case parser::LiteralNode::INTEGER_LITERAL: case parser::LiteralNode::HEXADECIMAL_LITERAL: {
				compiletime::Primitive p;
				

				if (node.type == parser::LiteralNode::INTEGER_LITERAL) {
					p.type = globalEnv->getType("int32");
					p.data.int32V = std::stoi(node.val);
				}
				else {
					p.type = globalEnv->getType("uint32");
					auto tmpVal = node.val;
					std::transform(tmpVal.begin(), tmpVal.end(), tmpVal.begin(), ::tolower);

					auto pos = tmpVal.find("0x");
					if (pos != std::string::npos) {
						tmpVal = tmpVal.substr(pos+2);
					}

					p.data.int32V = std::stoi(tmpVal,nullptr, 16);
				}
				


				std::string name = generateTempInstanceName(&node);
				currentModule->getCurrentScope()->addPrimLiteral(name, p);

				compiletime::Instance newInst;
				initCodeElement(newInst, name, node);				
				newInst.typeDecl = p.type;
				
					
				newInst.value.primitivePtr = &currentModule->getCurrentScope()->getPrimLiteral(name);
				currentModule->getCurrentScope()->addInstance(newInst);

			}break;

			case parser::LiteralNode::DECIMAL_LITERAL: {
				compiletime::Primitive p;
				p.type = globalEnv->getType("real64");
				p.data.doubleV = std::stod(node.val);


				std::string name = generateTempInstanceName(&node);
				currentModule->getCurrentScope()->addPrimLiteral(name, p);

				compiletime::Instance newInst;
				initCodeElement(newInst, name, node);
				newInst.typeDecl = p.type;
				

				newInst.value.primitivePtr = &currentModule->getCurrentScope()->getPrimLiteral(name);
				currentModule->getCurrentScope()->addInstance(newInst);

			}break;

			default: {
				throw Compiler::Error(*this, "Unknown literal type");
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
		initCodeElement(newInst, node.name, node);
		
		newInst.typeDecl = globalEnv->getType("message", false);
		currentModule->getCurrentScope()->addInstance(newInst);
	}

	void Compiler::stage1Instance(const parser::InstanceNode& node)
	{
		if (options.debugMode) {
			std::cout << "instance: " << node.getFullPath() << ":" << node.name << std::endl;
		}
		compiletime::Instance newInst;
		initCodeElement(newInst, node.name, node);
		bool newVarNeeded = true;
		if (!node.type.empty()) {
			newInst.typeDecl = currentModule->getCurrentScope()->getType(node.type, true);
		}
		else {
			auto v = currentModule->getCurrentScope()->getVariable(node.name,true);
			if (!v.empty()) {
				newInst.typeDecl = v.typeDecl;
				newVarNeeded = false;
			}
		}
		if (compiletime::TypeDescriptor::typeUnknown == newInst.typeDecl.type) {
			newInst.typeDecl.setName(node.type);
		}
			
		if (currentModule->getCurrentScope()->hasVariable(newInst.getName())) {
			newInst.variable = &currentModule->getCurrentScope()->getVariable(newInst.getName());
		}

		currentModule->getCurrentScope()->addInstance(newInst);


		if (!currentModule->getCurrentScope()->hasVariable(newInst.getName()) && newVarNeeded) {
			compiletime::Variable newVar;
			initCodeElement(newVar, newInst.getName(), node);
			newVar.typeDecl = newInst.typeDecl;

			
			if (!currentModule->getCurrentScope()->addVariable(newVar)) {				
				throw Compiler::Error(*this, "Variable already exists", VARIABLE_ALREADY_EXISTS, &newVar);

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
		llvmVariableBuilderPtr = std::make_unique<llvm::IRBuilder<>>(*llvmCtxPtr);

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

	void Compiler::initCompiletimeCustomClass(compiletime::Class& clazz)
	{
		auto assignment = [this](const compiletime::Instance* self, const std::vector<const compiletime::Instance*>& params) {

			return this->objectAssignment(self, params, *this->currentModule->getCurrentScope());
			};
		
		


		if (clazz.name == "module") {
			
		}
		else {
			clazz.addInstanceMethod(":=", ":=", assignment);
		}
	}

	void Compiler::initCompiletimeCustomRecord( compiletime::Class& clazz)
	{

	}

	void Compiler::initCompiletimeClass(compiletime::TypeDescriptor type, const std::string& name)
	{

		auto clazz = compiletime::Compiletime::getClass(name);
		if ( nullptr == clazz) {
			return;
		}

		switch (type) {
			case compiletime::TypeDescriptor::typeInteger8: case compiletime::TypeDescriptor::typeUInteger8:
			case compiletime::TypeDescriptor::typeInteger16: case compiletime::TypeDescriptor::typeUInteger16:
			case compiletime::TypeDescriptor::typeInteger32: case compiletime::TypeDescriptor::typeUInteger32:
			case compiletime::TypeDescriptor::typeInteger64: case compiletime::TypeDescriptor::typeUInteger64: {

				auto assignment = [this](const compiletime::Instance* self, const std::vector<const compiletime::Instance*>& params) {

					return this->primitiveAssignment(self, params, *self->getScope());
					};
				clazz->addInstanceMethod(":=", ":=", assignment);
			
			}break;

			case compiletime::TypeDescriptor::typeInteger128: case compiletime::TypeDescriptor::typeUInteger128: {

				auto assignment = [this](const compiletime::Instance* self, const std::vector<const compiletime::Instance*>& params) {

					return this->primitiveAssignment(self, params, *self->getScope());
					};
				clazz->addInstanceMethod(":=", ":=", assignment);
			}break;
														   

			case compiletime::TypeDescriptor::typeDouble32: {

				auto assignment = [this](const compiletime::Instance* self, const std::vector<const compiletime::Instance*>& params) {

					return this->primitiveAssignment(self, params, *self->getScope());
					};
				clazz->addInstanceMethod(":=", ":=", assignment);
			}break;

			case compiletime::TypeDescriptor::typeDouble64: {

				auto assignment = [this](const compiletime::Instance* self, const std::vector<const compiletime::Instance*>& params) {

					return this->primitiveAssignment(self, params, *self->getScope());
					};
				clazz->addInstanceMethod(":=", ":=", assignment);
			}break;


			case compiletime::TypeDescriptor::typeString: {

				auto assignment = [this](const compiletime::Instance* self, const std::vector<const compiletime::Instance*>& params) {

					return this->primitiveStringAssignment (self, params, *self->getScope());
					};
				clazz->addInstanceMethod(":=", ":=", assignment);
			}break;


			case compiletime::TypeDescriptor::typeClass: {
				initCompiletimeCustomClass(*clazz);
			}break;

			case compiletime::TypeDescriptor::typeRecord: {
				initCompiletimeCustomRecord(*clazz);

			}break;

			default: {
				Compiler::println("Warning: compile time class '" + name + "' not handled");
			}break;
		}
	}

	void Compiler::initBasicTypes()
	{
		static std::map<compiletime::TypeDescriptor, std::string> globalPrims = compiletime::Compiletime::globalPrims();

		for (auto t : globalPrims) {
				
			if (!globalEnv->addType(compiletime::TypeDecl(t.second, t.first))) {
				throw Compiler::Error(*this, "Type already exists");
			}
			initCompiletimeClass(t.first, t.second);
		}



		//custom classes
		if (!globalEnv->addType(compiletime::TypeDecl("module", compiletime::TypeDescriptor::typeClass)) ) {
			throw Compiler::Error(*this, "Type already exists");
		}
		else {
			initCompiletimeClass(compiletime::TypeDescriptor::typeClass,"module");
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
		compiletime::Compiletime::terminate();

		delete globalEnv;
	}


	void Compiler::printCodeObjModel()
	{
		Compiler::println("printCodeObjModel:");
		
		Compiler::println("Program: '" + executableCodePtr->getFullyQualifiedName() +"' version: '" + executableCodePtr->version + "'");
		


		for (auto m : executableCodePtr->modules) {
			auto& module = *m.second;

			module.resetScope();

			Compiler::println("module: '" + module.getFullyQualifiedName() + "' version: '" + module.version + "'");
			std::string indent = "\t";
			std::function<void(const compiletime::NamespaceBlock& ns, std::string& )> nsFunc;

			std::function<void(const compiletime::ScopedEnvironment& scope, std::string&, bool doChildren)> scopeFunc;

			scopeFunc = ([&scopeFunc, this](const compiletime::ScopedEnvironment& scope, std::string& indent, bool doChildren) -> void {
				Compiler::println(indent + "vars:");
				auto tmpI = indent + "\t";
				for (auto v : scope.getVariables()) {
					Compiler::println(tmpI + "var: '" + v.second.getFullyQualifiedName() + "', type: '" + v.second.typeDecl.fullyQualifiedName() + "'");
				}

				Compiler::println(indent + "expressions:");

				for (const auto& msg : scope.getMsgSends()) {
					const auto& sendMsg = msg.second;
					Compiler::println(tmpI + "recvr: '" + sendMsg.instance->getFullyQualifiedName()+"'");
					Compiler::println(tmpI + "selector: '" + sendMsg.getSelector().name + "'");
					auto tmp2 = tmpI + "\t";
					for (const auto& p : sendMsg.msg.parameters) {
						const auto& msgParam = p.second;
						Compiler::println(tmp2 + "param: '" + msgParam.getFullyQualifiedName() + "'");
						if (nullptr == msgParam.param) {
							Compiler::println(tmp2+"\t" + "param instance NULL!'");
						}
					}					 
				}

				if (doChildren) {
					for (const auto& s : scope.children) {
						scopeFunc(*s, indent, doChildren);
					}
				}

				indent += "\t";
			});

			nsFunc = ([&nsFunc, this, &scopeFunc](const compiletime::NamespaceBlock& ns, std::string& indent) -> void {
				
				Compiler::println(indent + "namespace: '" + ns.getFullyQualifiedName() + "'");

				if (nullptr == ns.getScope()) {
					throw Compiler::Error(*this, "Namespace '" + ns.getName() + "' has no scope!", OBJECT_HAS_NO_SCOPE, &ns);
				}

				for (const auto& n : ns.namespaces) {
					nsFunc(n.second, indent);					
				}

				auto tmpI = indent + "\t";
				

				for (const auto& e: ns.code.extendsDefs) {
					const auto& extension = e.second;
					Compiler::println(tmpI + "extension for: '" + extension.extensionTarget + "'");					
					Compiler::println(tmpI + "messages:");

					tmpI += "\t";
					for (auto msg : extension.messages) {
						const auto& msgDecl = msg.second;
						Compiler::println(tmpI + "msg: '" + msgDecl.getFullyQualifiedName() + "'");
						auto tmp2 = tmpI+"\t";

						for (auto p: msgDecl.parameters) {
							const auto& param = p.second;
							Compiler::println(tmp2 + "param:'" + param.getName() + "', type: '" + param.typeDecl.fullyQualifiedName() + "'");
						}
						
						Compiler::println(tmpI + "msg impl body:");
						if (nullptr == msgDecl.impl) {
							Compiler::println(tmpI + "\t" + "msg decl impl is NULL!");
						}
						auto impl = extension.implementations.find(msgDecl.getName());
						if (impl != extension.implementations.end()) {
							const auto& msgImpl = impl->second;							
							auto tmp3 = tmp2;
							scopeFunc(*msgImpl.getScope(), tmp3,true);

							Compiler::println(tmp2 + "returns: " + ((msgImpl.returnVal != NULL) ? msgImpl.returnVal->typeDecl.getFullyQualifiedName() : "N/A"));
							
						}
						
					}
				}

				auto nsScope = ns.getScope();
				
				scopeFunc(*nsScope, indent,false);

				indent += "\t";
			});


			if (nullptr == module.getGlobalNamespace()->getScope()) {
				throw Compiler::Error(*this, "Module '" + module.getName() + "' global namespace has no scope!", OBJECT_HAS_NO_SCOPE, module.getGlobalNamespace());
			}

			nsFunc(*module.getGlobalNamespace(), indent);
		}


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
				try {
					pass.run(m);
				}
				catch (std::exception& e) {
					throw Compiler::Error(*this, e.what());
				}
				
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
		std::string link_command;

		//if (options.debugMode) {
			link_command += "/verbose ";
		//}

		link_command += "/subsystem:CONSOLE /nodefaultlib /entry:" + Compiler::mainEntryFunction() + " ";			


		if (outputFileName.find(".exe")==std::string::npos) {
			outputFileName += ".exe";
		}

		if (std::filesystem::exists(outputFileName)) {
			std::filesystem::remove(outputFileName);
		}

		link_command += "/out:" + outputFileName + " ";

#ifdef SCRATCH_RUNTIME_DEBUG
		objs.push_back("scratch-lang-runtime/objs/scratch_runtime_x64_Debug.obj");
		objs.push_back("scratch-lang-runtime/objs/rt_lang_memcpy_x64_Debug.obj");
#else 
		objs.push_back("scratch-lang-runtime/objs/scratch_runtime_x64_Release.obj");
		objs.push_back("scratch-lang-runtime/objs/rt_lang_memcpy_x64_Release.obj");
#endif

		for (auto obj : objs) {
			link_command += obj + " ";
		}


			
		std::vector<std::string> win32CRTLibs = { "kernel32.lib", "user32.lib", "Shell32.lib"};
	
		for (auto lib : win32CRTLibs) {
			link_command += lib + " ";
		}

		if (options.debugMode) {
			Compiler::println( "command: lld-link " + link_command );
		}

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
	

