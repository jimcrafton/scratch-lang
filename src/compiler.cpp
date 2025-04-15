#include "Compiler.h"
#include "AST.h"

#include "Lexer.h"
#include "Parser.h"

#include <utility>
#include <cstdlib>
#include <fstream>
#include <algorithm>

#include <iostream>

#include "cmd_line_options.h"
#include "version.h"

namespace language {
	namespace compiler {

		class Stage1 : public language::AstVisitor {
		public:
			virtual ~Stage1() {}

			Compiler& compiler;
			Stage1(Compiler& c) :compiler(c) {
				
			}

			virtual void visitNilNode(const parser::NilNode& node) {
				if (state == language::AstVisitor::stateVisitStarted) {
					std::cout << "visitNilNode" << std::endl;
				}
				
			}

			virtual void visitVariableNode(const parser::VariableNode& node) {
				if (state == language::AstVisitor::stateVisitStarted) {
					compiler.stage1Variable(node);
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

			virtual void visitAssignment(const parser::Assignment& node) {
				if (state == language::AstVisitor::stateVisitStarted) {
					compiler.stage1NewSendMessage(node);
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
			auto o = cmdline["compile-only"];
			if (!o.is_null()) { compileOnly = o; }
			
			o = cmdline["verbose-mode"];
			if (!o.is_null()) { verboseMode = o; }

			o = cmdline["debug-mode"];
			if (!o.is_null()) { debugMode = o; }

			o = cmdline["print-ast"];
			if (!o.is_null()) { printAST = true; }
			
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

		void Compiler::build(const std::vector<std::string>& files)
		{
			if (files.empty()) {
				throw Compiler::Error(*this, "no input files");
			}
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
					compile(parser.ast);
				}
				
			}
			
			if (options.printAST) {
				return;
			}

			createMainEntryPoint();


			Linker linker;

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
				Compiler::println("linking executable " + executableCodePtr->name + " - " + executableCodePtr->version);
				linker.outputFileName = executableCodePtr->name;

				linker.link(*this);
			}
			else {
				Compiler::println("linking disabled");
			}

			if (executableCodePtr) {
				executableCodePtr.reset();
			}
		}

		void Compiler::compile(const language::AST& ast)
		{
			stage1(ast);

			

			/*
			std::function<void(const ScopedEnvironment& scope)> func;
			
			func = ([&func](const ScopedEnvironment& scope ) -> void{
				std::string tab = "";// "\t";
				auto p = scope.parent;
				while (p) {
					p = p->parent;
					tab += "\t";
				}
				std::cout << tab << "scope " << scope.children.size() << " children" << std::endl;
				std::cout << tab << "\ttypes" << std::endl;
				for (const auto& t : scope.getTypes()) {
					std::cout << tab << "\t" << t.second.name << " " << t.second.type << std::endl;
				}
				std::cout << tab << "\tinstances" << std::endl;
				for (const auto& i : scope.getInstances()) {
					std::cout << tab << "\t" << i.second.name << " " << i.second.type.type << std::endl;
				}

				for (auto s : scope.children) {
					func(*s);
				}
			});
			

			func(globalEnv);
			*/
		}

		void Compiler::dumpCompilerOutput()
		{
			//modulePtr->print(llvm::outs(), nullptr);
		}


		void Compiler::dumpCompilerObjectCode()
		{
			
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

		void Compiler::createMainEntryPoint()
		{
			
			std::string mainFuncName = "main";
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

			auto mainReturnCode = 1221;
			auto retVal = llvmBuilderPtr->getInt32(mainReturnCode);
			auto i32Res = llvmBuilderPtr->CreateIntCast(retVal, llvmBuilderPtr->getInt32Ty(), true);
			llvmBuilderPtr->CreateRet(i32Res);
			

		}

		void Compiler::stage1(const language::AST& ast)
		{ 
		
			currentScope = &globalEnv;

			Stage1 s1(*this);
			ast.visitAll(s1);
			//ast.reverseVisitAll(s1);

			closeCurrentScope();




			if (!executableCodePtr) {
				programInst = new Program();
				programInst->name = "a";
				executableCodePtr.reset(programInst);
				Module* m = new Module(*this);
				executableCodePtr->modules.insert(std::make_pair(m->name, m));
			}

		}

		void Compiler::stage1NewModule(const parser::ModuleBlock& node)
		{
			if (!executableCodePtr) {
				programInst = new Program();
				programInst->name = "a";
				executableCodePtr.reset(programInst);
			}


			Module* m = new Module(*this);
			m->name = node.name;
			m->version = node.version;
			executableCodePtr->modules.insert(std::make_pair(m->name, m));

			if (options.debugMode) {
				std::cout << "ModuleBlock " << node.name << std::endl;
			}
		}

		void Compiler::stage1NewClass(const parser::ClassBlock& node)
		{
			pushNewScope();

			Type newType;
			newType.name = node.name;
			newType.type = Type::typeClass;


			currentScope->addType(newType);
		}

		void Compiler::stage1CloseClassBlock(const parser::ClassBlock& node)
		{
			closeCurrentScope();
		}

		void Compiler::closeScope(const ScopedEnvironment& scope)
		{
			if (options.debugMode) {
				scope.debugPrint();
			}
		}

		void Compiler::closeCurrentScope() 
		{
			auto scopePtr = popCurrentScope();
			if (nullptr != scopePtr) {
				closeScope(*scopePtr);
			}
		}

		ScopedEnvironment* Compiler::popCurrentScope()
		{
			ScopedEnvironment* result = currentScope;
			auto parent = currentScope->parent;
			currentScope = parent;
			if (nullptr == currentScope) {
				currentScope = &globalEnv;
			}
			return result;
		}

		ScopedEnvironment* Compiler::createNewScope()
		{
			ScopedEnvironment* result = new ScopedEnvironment(*this);
			currentScope->add(result);
			return result;
		}

		ScopedEnvironment* Compiler::pushNewScope()
		{
			ScopedEnvironment* newScope = createNewScope();
			currentScope = newScope;
			return currentScope;
		}

		void Compiler::stage1NewMessageBlock(const parser::MessageBlock& node)
		{
			pushNewScope();
		}

		void Compiler::stage1CloseMessageBlock(const parser::MessageBlock& node)
		{
			if (options.debugMode) {
				std::cout << "closing message " << node.name << std::endl;
			}
			
			closeCurrentScope();
		}

		void Compiler::stage1NewSendMessage(const parser::SendMessage& node)
		{
			//std::cout << "stage1NewSendMessage " << node.message->name << std::endl;
			SendMessage newSendMsg;
			newSendMsg.msg.name = node.message->name;
			std::string instName = node.instance->name;
			if (instName.empty()) {
				char tmp[256] = { 0 };
				snprintf(tmp, sizeof(tmp) - 1, "%p", node.instance);
				instName = node.instance->getFullPath()+"_tmp_" + tmp;
			}
			if (currentScope->hasInstance(instName)) {
				newSendMsg.instance = &currentScope->getInstance(node.instance->name);
			}
			else {
				Instance newInst;
				newInst.name = instName;
				

				currentScope->addInstance(newInst);
				newSendMsg.instance = &currentScope->getInstance(instName);
				
			}
			
			for (auto p : node.message->parameters) {

			}
			currentScope->addSendMessage(newSendMsg);
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
		}

		void Compiler::stage1NewRecord(const parser::RecordBlock& node)
		{
			Type newType;
			newType.name = node.name;
			newType.type = Type::typeClass;

			currentScope->addType(newType);
		}

		void Compiler::stage1Variable(const parser::VariableNode& node)
		{
			if (options.debugMode) {
				std::cout << "variable: " << node.getFullPath() << ":" << node.name << std::endl;
			}
			

			Variable newVar;
			newVar.name = node.name;
			if (!node.type.empty()) {
				newVar.type = currentScope->getType(node.type, true);
			}
			currentScope->addVariable(newVar);
		}

		void Compiler::stage1Instance(const parser::InstanceNode& node)
		{
			if (options.debugMode) {
				std::cout << "instance: " << node.getFullPath() << ":" << node.name << std::endl;
			}
			Instance newInst;
			newInst.name = node.name;
			if (!node.type.empty()) {
				newInst.type = currentScope->getType(node.type, true);
			}			
			
			currentScope->addInstance(newInst);
		}


		void Compiler::clearEnvironment()
		{
			globalEnv.clear();
			classes.clear();
			programInst = nullptr;
			libInst = nullptr;
			executableCodePtr.reset();
			currentScope = &globalEnv;
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

			initBasicTypes();
		}

	
		void Compiler::initBasicTypes()
		{
			static std::map<Type::TypeOf, std::string> globalPrims = {
				{Type::typeInteger8, "int8"},
				{Type::typeUInteger8, "uint8"},
				{Type::typeInteger16, "int16"},
				{Type::typeUInteger16, "uint16"},
				{Type::typeInteger32, "int32"},
				{Type::typeUInteger32, "uint32"},
				{Type::typeInteger64, "int64"},
				{Type::typeUInteger64, "uint64"},
				{Type::typeInteger128, "int128"},
				{Type::typeUInteger128, "uint128"},
				{Type::typeDouble32, "real32"},
				{Type::typeDouble64, "real64"},
				{Type::typeDouble64, "bool"},
				{Type::typeDouble64, "string"},
				{Type::typeDouble64, "array"},
				{Type::typeDouble64, "dictionary"}
			};

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
					outfileName = executableCodePtr->name + "_main_entry.ll";
				}
				break;

				case OUT_OBJECT: {
					outfileName = executableCodePtr->name + "_main_entry.o";
				}
				break;
			}
			outputModule(outFmt,*mainEntryModulePtr, outfileName);
		}

		void Module::init(Compiler& c)
		{
			modulePtr.reset( c.createModule(name) );
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
			
			c.outputModule(outFmt, *modulePtr, outputFileName);
		}


		void ScopedEnvironment::addType(const Type& t)
		{
			auto name = t.fullyQualifiedName();
			if (hasType(name)) {
				throw Compiler::Error(compiler, "Type already exists");
			}
			types.insert(std::make_pair(name, t));
		}

		void ScopedEnvironment::addInstance(const Instance& i)
		{
			if (hasInstance(i.name)) {
				//throw Compiler::Error(compiler, "Instance already exists");
				return;
			}
			
			auto res = instances.insert(std::make_pair(i.name, i));
			res.first->second.scope = this;
		}

		void ScopedEnvironment::addVariable(const Variable& v)
		{
			if (hasVariable(v.name)) {
				throw Compiler::Error(compiler, "Variable already exists");
			}

			auto res = variables.insert(std::make_pair(v.name, v));
			res.first->second.scope = this;
		}

		void ScopedEnvironment::addSendMessage(const SendMessage& msg)
		{
			std::string key = msg.msg.name + "_to_" + (msg.instance!=nullptr ? msg.instance->name : std::string("null"));
			if (!msg.msg.parameters.empty()) {
				key += "_with_";
				for (auto p : msg.msg.parameters) {

				}
			}
			auto res = msgSends.insert(std::make_pair(key, msg));
			res.first->second.scope = this;
		}

		void Linker::init()
		{

		}

		void Linker::terminate()
		{

		}

		void Linker::link(const Compiler& compiler)
		{
			//run llvm linker
			std::string link_command = "lld-link /entry:main ";

			if (outputFileName.find(".exe")==std::string::npos) {
				outputFileName += ".exe";
			}

			link_command += "/out:" + outputFileName + " ";
			for (auto obj : objs) {
				link_command += obj + " ";
			}
			
			//link_command += " > tmp.out 2>&1";
			//std::cout.flush();
			std::system(link_command.c_str());
			//std::cout << std::ifstream("tmp.out").rdbuf();
			//std::cout.flush();

			Compiler::println("executable: " + outputFileName);
		}
	}
	
}
