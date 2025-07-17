#include "Compiler.h"
#include "AST.h"

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




namespace language {
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


		



		llvm::Value* Instance::getValue(llvm::IRBuilder<>* builder) const
		{
			llvm::Value* result = nullptr;

			switch (type.type) {
				case language::compiletime::TypeDescriptor::typeBool: {
					result = value.primitivePtr->data.boolV ? builder->getTrue() : builder->getFalse();
				}break;

				case language::compiletime::TypeDescriptor::typeInteger8: {
					result = builder->getInt8(value.primitivePtr->data.int8V);
				}break;

				case language::compiletime::TypeDescriptor::typeUInteger8: {

					result = builder->getInt8(value.primitivePtr->data.uint8V);
				}break;

				case language::compiletime::TypeDescriptor::typeUInteger32: {

					result = builder->getInt32 (value.primitivePtr->data.uint32V);
				}break;

				case language::compiletime::TypeDescriptor::typeInteger32: {

					result = builder->getInt32(value.primitivePtr->data.int32V);
				}break;

				case language::compiletime::TypeDescriptor::typeString: {

				}break;

				default: {
					result = nullptr;
				}break;
			}

			return result;
		}


		void Compiler::Error::output() const 
		{
			std::cout << "Compiler error: " << message << std::endl;
			compiler.outputErrors();
		}

		Compiler::Compiler(const utils::cmd_line_options& cmdline) :
			cmdlineOpts(cmdline),
			globalEnv(*this)
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
				programInst = new Program();
				programInst->setName("a");
				executableCodePtr.reset(programInst);
				Module* m = new Module(*this);
				executableCodePtr->modules.insert(std::make_pair(m->getName(), m));
			}

			stage3();
		}

		void Compiler::link()
		{
			Linker linker(cmdlineOpts);

			if (executableCodePtr) {
				for (auto m : executableCodePtr->modules) {
					auto module = m.second;
					std::string objName;
					module->output(*this, OUT_INTERMEDIATE, objName);

					module->output(*this, OUT_OBJECT, objName);
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
			return "runtime_main";
		}

		void Compiler::createMainEntryPoint()
		{
			
			std::string mainFuncName = Compiler::mainEntryFunction();
			auto returnType = llvm::FunctionType::get(llvmBuilderPtr->getInt32Ty(), false);
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

			auto mainReturnCode = 0;
			auto retVal = llvmBuilderPtr->getInt32(mainReturnCode);
			auto i32Res = llvmBuilderPtr->CreateIntCast(retVal, llvmBuilderPtr->getInt32Ty(), true);
			llvmBuilderPtr->CreateRet(i32Res);
			

		}

		void Compiler::stage1(const language::AST& ast)
		{ 
			if (nullptr == executableCodePtr) {
				programInst = new Program();
				programInst->setName("a");
				executableCodePtr.reset(programInst);
				Module* m = new Module(*this);
				executableCodePtr->modules.insert(std::make_pair(m->getName(), m));
				m->resetScope();

				currentModule = m;
			}

			Stage1 s1(*this);
			ast.visitAll(s1);

			
			currentModule->closeCurrentScope();

			if (options.debugMode) {
				globalEnv.debugPrint();
			}
			
		}

		void Compiler::stage2 ()
		{
			//verify var types
			


		
			std::function<void(ScopedEnvironment& scope)> func;

			func = ([&func,this](ScopedEnvironment& scope) -> void {
				
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
						if (var.type.type == language::compiletime::TypeDescriptor::typeUnknown) {
							Variable resolvedVar;
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

		

		void Compiler::buildFunction(const Instance* receiver, const CppString& selector, const std::vector<const Instance*>& params , const ScopedEnvironment& scope)
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


		void Compiler::buildFunction(const Instance* receiver, const Message& msg, const ScopedEnvironment& scope)
		{
			auto selector = msg.getName();

			std::vector<const Instance*> params;

			for (const auto& p:msg.parameters) {
				params.push_back(p.second.param);
			}

			buildFunction(receiver, selector, params, scope);
		}

		void Compiler::buildFunctions( const ScopedEnvironment& scope)
		{
			for (const auto& m : scope.getMsgSends()) {
				const SendMessage& sendMsg = m.second;

				//1) reciever
				//2) message selector (name of the message)
				//3) 0 or more parameters

				const Instance* receiver = nullptr;

				receiver = sendMsg.instance;

				buildFunction(receiver, sendMsg.msg, scope);
			}
		}

		void Compiler::stage3()
		{
			

			std::function<void(ScopedEnvironment& scope)> func;
			func = ([&func, this](ScopedEnvironment& scope) -> void {
				for (auto revIt = scope.children.rbegin(); revIt != scope.children.rend(); ++revIt) {
					auto s = *revIt;
					func(*s);
				}

				llvm::Value* val = nullptr;
				for (const auto& i : scope.getInstances()) {
					const Instance& inst = i.second;
					
				}
				for (const auto& v : scope.getVariables()) {
					const Variable& var = v.second;
					auto varName = var.getName();
					switch (var.type.type) {
						case language::compiletime::TypeDescriptor::typeInteger8 : {
							
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
				Module* m = new Module(*this);
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
			Type newType;
			newType.setName(node.name);
			newType.type = language::compiletime::TypeDescriptor::typeClass;
			newType.setLocation(node.token.location);
			currentModule->getCurrentScope()->addType(newType);

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
			SendMessage newSendMsg;
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
				Instance newInst;
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
			SendMessage newSendMsg;
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
				Variable newVar;
				newVar.setName(p->name);
				newVar.setLocation(p->token.location);
				if (!p->type.empty()) {
					newVar.type = currentModule->getCurrentScope()->getType(p->type, true);
				}
				if (language::compiletime::TypeDescriptor::typeUnknown == newVar.type.type) {
					newVar.type.setName(p->type);
				}
				currentModule->getCurrentScope()->addVariable(newVar);
			}
		}

		void Compiler::stage1NewRecord(const parser::RecordBlock& node)
		{
			Type newType;
			newType.setName(node.name);
			newType.type = language::compiletime::TypeDescriptor::typeClass;
			newType.setLocation(node.token.location);

			currentModule->getCurrentScope()->addType(newType);
		}

		void Compiler::stage1CloseRecord(const parser::RecordBlock & node)
		{

		}

		void Compiler::stage1Variable(const parser::VariableNode& node)
		{
			if (options.debugMode) {
				std::cout << "variable: " << node.getFullPath() << ":" << node.name << std::endl;
			}
			

			Variable newVar;
			newVar.setName( node.name);
			newVar.setLocation(node.token.location);
			if (!node.type.empty()) {

				if (!currentModule->getCurrentScope()->hasType(node.type, true)) {
					printf("ERROR! No type (%s) found!\n", node.type.c_str());
				}

				newVar.type = currentModule->getCurrentScope()->getType(node.type, true);
			}
			if (language::compiletime::TypeDescriptor::typeUnknown == newVar.type.type) {
				newVar.type.setName(node.type);
			}
			currentModule->getCurrentScope()->addVariable(newVar);
		}

		void Compiler::stage1Literal(const parser::LiteralNode& node)
		{
			if (options.debugMode) {
				std::cout << "literal: " << node.getFullPath() << ": '" << node.val << "', " << node.type << std::endl;
			}

			switch (node.type) {
				case parser::LiteralNode::STRING_LITERAL: {
					language::runtime::datatypes::string s;
					s.assign(node.val);
					std::string name = generateTempInstanceName(&node);
					currentModule->getCurrentScope()->addStringLiteral(name, s);
					
					Instance newInst;
					newInst.setName(name);
					newInst.type = currentModule->getCurrentScope()->getType("string", true);
					newInst.setLocation(node.token.location);
					
					newInst.value.stringPtr = &currentModule->getCurrentScope()->getStringLiteral(name);
					currentModule->getCurrentScope()->addInstance(newInst);

				}break;

				case parser::LiteralNode::INTEGER_LITERAL: {
					Primitive p;
					p.type = globalEnv.getType("int32");
					p.data.int32V = std::stoi(node.val);


					std::string name = generateTempInstanceName(&node);
					currentModule->getCurrentScope()->addPrimLiteral(name, p);

					Instance newInst;
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


			Instance newInst;
			newInst.setName(node.name);
			newInst.setLocation(node.token.location);
			newInst.type = globalEnv.getType("message", false);
			currentModule->getCurrentScope()->addInstance(newInst);
		}

		void Compiler::stage1Instance(const parser::InstanceNode& node)
		{
			if (options.debugMode) {
				std::cout << "instance: " << node.getFullPath() << ":" << node.name << std::endl;
			}
			Instance newInst;
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
			if (language::compiletime::TypeDescriptor::typeUnknown == newInst.type.type) {
				newInst.type.setName(node.type);
			}
			
			currentModule->getCurrentScope()->addInstance(newInst);

			if (!currentModule->getCurrentScope()->hasVariable(newInst.getName()) && newVarNeeded) {
				Variable newVar;
				newVar.setName(newInst.getName());
				newVar.type = newInst.type;
				newVar.setLocation(node.token.location);

				currentModule->getCurrentScope()->addVariable(newVar);
			}			
		}


		void Compiler::clearEnvironment()
		{
			buildStart = buildEnd = std::chrono::system_clock::now();

			globalEnv.clear();
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


			language::compiletime::Compiletime::init();

			initBasicTypes();
		}

	
		void Compiler::initBasicTypes()
		{
			static std::map<language::compiletime::TypeDescriptor, std::string> globalPrims = language::compiletime::Compiletime::globalPrims();

			for (auto t : globalPrims) {
				
				globalEnv.addType(Type(t.second, t.first));
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

		Module::~Module()
		{
			
		}

		void Module::init(Compiler& c)
		{
			name = "a";

			llvmModulePtr.reset( c.createModule(name) );

			globalEnv = new ScopedEnvironment(c);
			globalEnv->name = "module scope " + name;
			
			c.getGlobalScope()->add(globalEnv);
		}

		void Module::output(Compiler& c, OutputFormat outFmt, std::string& outputFileName)
		{
			//to file
			outputFileName = name + "_";
			
			auto tmpv = version;
			std::replace(tmpv.begin(), tmpv.end(), '.', '_');
			outputFileName += tmpv;

			switch (outFmt) {
				case OUT_INTERMEDIATE: {
					outputFileName += ".ll";
				}
				break;

				case OUT_OBJECT: {
					outputFileName += ".o";
				}
				break;
			}
			
			c.outputModule(outFmt, *llvmModulePtr, outputFileName);
		}

		void Module::resetScope()
		{
			if (nullptr == globalEnv) {
				globalEnv = new ScopedEnvironment(compiler);
				compiler.getGlobalScope()->add(globalEnv);
			}
			currentScope = globalEnv;
		}

		void Module::closeScope(const ScopedEnvironment& scope)
		{
			if (compiler.options.debugMode) {
				scope.debugPrint();
			}
		}

		void Module::closeCurrentScope() {
			auto scopePtr = popCurrentScope();
			if (nullptr != scopePtr) {
				closeScope(*scopePtr);
			}
		}

		ScopedEnvironment* Module::popCurrentScope() {
			ScopedEnvironment* result = currentScope;
			auto parent = currentScope->parent;
			currentScope = parent;
			if (nullptr == currentScope) {
				currentScope = globalEnv;
			}
			return result;
		}

		ScopedEnvironment* Module::createNewScope() {
			ScopedEnvironment* result = new ScopedEnvironment(compiler);
			currentScope->add(result);
			return result;
		}

		ScopedEnvironment* Module::pushNewScope() {
			ScopedEnvironment* newScope = createNewScope();
			currentScope = newScope;
			return currentScope;
		}


		void ScopedEnvironment::addType(const Type& t)
		{
			auto name = t.fullyQualifiedName();
			if (hasType(name)) {
				throw Compiler::Error(compiler, "Type already exists");
			}
			auto res = types.insert(std::make_pair(name, t));
			res.first->second.setScope(this);

		}

		CppString ScopedEnvironment::getFullName() const
		{
			CppString result;
			
			auto p = parent;
			while (p) {
				if (nullptr != p) {
					if (!result.empty()) {
						result = "." + result;
					}

					std::string pname = (p->name.empty() && p->parent == nullptr) ? "global" : p->name;
					result = pname + result;
				}
				p = p->parent;
			}

			if (!result.empty()) {
				result += ".";
			}
			result += name;

			if (result.empty() && nullptr == parent) {
				result = "global";
			}

			return result;
		}

		void ScopedEnvironment::debugPrint() const
		{
			std::string tab = "";// "\t";
			auto p = parent;
			while (p) {
				p = p->parent;
				tab += "\t";
			}

			std::string line;
			line.append(80, '-');

			std::cout << tab << "scope: '" << getFullName() << "'" << std::endl;
			std::cout << tab << "\ttypes" << std::endl;

			for (const auto& t : getTypes()) {
				if (t.second.isPrimitive()) {
					std::cout << tab << "\t" << t.second.getName() << " " << t.second.type << std::endl;
				}
				else {
					std::cout << tab << "\t * " << t.second.getName() << " " << t.second.type << std::endl;
				}

			}
			std::cout << line << std::endl;
			std::cout << tab << "\tinstances" << std::endl;
			for (const auto& i : getInstances()) {
				std::cout << tab << "\t" << i.second.getName() << " " << i.second.type.type << std::endl;
			}
			std::cout << line << std::endl;
			std::cout << tab << "\tvariables" << std::endl;
			for (const auto& v : getVariables()) {
				std::cout << tab << "\t" << v.second.getName() << " '" << v.second.type.fullyQualifiedName() << "' : " << v.second.type.type << std::endl;
			}
			std::cout << line << std::endl;
			std::cout << tab << "\tsend messages" << std::endl;
			for (const auto& msg : getMsgSends()) {
				std::cout << tab << "\t" << msg.first /* << " " << v.second.type.fullyQualifiedName() << ":" << v.second.type.type*/ << std::endl;
			}
		}

		void ScopedEnvironment::addInstance(const Instance& i)
		{
			if (hasInstance(i.getName())) {
				//throw Compiler::Error(compiler, "Instance already exists");
				return;
			}
			
			auto res = instances.insert(std::make_pair(i.getName(), i));
			res.first->second.setScope(this);
		}

		void ScopedEnvironment::addVariable(const Variable& v)
		{
			if (hasVariable(v.getName())) {
				throw Compiler::Error(compiler, "Variable already exists");
			}

			printf("'%s' : '%s'\n", v.getName().c_str(), v.type.getName().c_str());

			auto res = variables.insert(std::make_pair(v.getName(), v));
			res.first->second.setScope(this);
		}

		void ScopedEnvironment::addSendMessage(const SendMessage& msg)
		{
			std::string key = msg.msg.getName() + "_to_" + (msg.instance != nullptr ? msg.instance->getName() : std::string("null"));
			if (!msg.msg.parameters.empty()) {
				key += "_with_";
				for (auto p : msg.msg.parameters) {

				}
			}
			auto res = msgSends.insert(std::make_pair(key, msg));
			res.first->second.setScope(this);
		}

		void ScopedEnvironment::addStringLiteral(const CppString& name, const language::runtime::datatypes::string& strlit)
		{
			if (strings.count(name) != 0) {
				return;
			}
			auto res = strings.insert(std::make_pair(name, strlit));
		}

		void ScopedEnvironment::addPrimLiteral(const CppString& name, const Primitive& prim)
		{
			if (primitives.count(name) != 0) {
				return;
			}
			auto res = primitives.insert(std::make_pair(name, prim));
		}

		bool ScopedEnvironment::resolveVariable(const Variable& var, Variable& outVar)
		{
			Variable* foundVar = nullptr;
			if (var.type.type != language::compiletime::TypeDescriptor::typeUnknown) {
				return true;
			}

			auto res = hasType(var.type.getName(), true);
			printf("%s (%s) has type %d\n", var.getName().c_str(), var.type.getName().c_str(), (int)res);
			if (res) {
				auto t = getType(var.type.getName(), true);
				auto p = this;
				while (nullptr != p) {
					auto found = p->variables.find(var.getName());
					if (found != p->variables.end()) {
						foundVar = &found->second;
						break;
					}
					p = p->parent;
				}

				if (nullptr != foundVar) {
					foundVar->type = t;
					outVar = *foundVar;
				}
			}
			return foundVar == nullptr ? false : true;
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
			std::string link_command = "/verbose /subsystem:console /entry:" + Compiler::mainEntryFunction() + " ";
			


			if (outputFileName.find(".exe")==std::string::npos) {
				outputFileName += ".exe";
			}

			if (std::filesystem::exists(outputFileName)) {
				std::filesystem::remove(outputFileName);
			}

			link_command += "/out:" + outputFileName + " ";
			for (auto obj : objs) {
				link_command += obj + " ";
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
	
}
