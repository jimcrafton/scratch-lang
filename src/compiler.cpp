#include "Compiler.h"
#include "AST.h"

#include "Lexer.h"
#include "Parser.h"

#include <utility>
#include <cstdlib>
#include "cmd_line_options.h"


namespace language {
	namespace compiler {

		class Stage1 : public AstVisitor {
		public:
			Compiler& compiler;
			Stage1(Compiler& c) :compiler(c) {
				
			}

			virtual void visitNilNode(const NilNode& node) {
				std::cout << "visitNilNode" << std::endl;
			}

			virtual void visitVariableNode(const VariableNode& node) {
				std::cout << "visitVariableNode " << node.name << std::endl;
			}

			virtual void visitModuleBlock(const ModuleBlock& node) {
				std::cout << "visitModuleBlock " << node.name << std::endl;

				if (!compiler.executableCodePtr) {
					compiler.programInst = new Program();
					compiler.programInst->name = "a";
					compiler.executableCodePtr.reset(compiler.programInst);
				}
				

				Module* m = new Module(compiler);
				m->name = node.name;
				compiler.executableCodePtr->modules.insert(std::make_pair(m->name, m));
			}
		};

		void CompilerOptions::init(utils::cmd_line_options& cmdline)
		{
			auto o = cmdline["compile-only"];
			if (!o.is_null()) { compileOnly = o; }
			
		}

		void Compiler::build(const std::vector<std::string>& files)
		{
			if (files.empty()) {
				throw Compiler::Error(*this, "no input files");
			}
			clearEnvironment();

			for (auto file : files) {
				Lexer lexer;

				lexer.lex(file);

				Parser parser;

				parser.parse(lexer);

				compile(parser.ast);
			}
			
			Linker linker;

			if (executableCodePtr) {
				for (auto m : executableCodePtr->modules) {
					auto module = m.second;
					std::string objName;
					module->output(*this, OUT_INTERMEDIATE, objName);
					module->output(*this, OUT_OBJECT, objName);
					if (!this->options.compileOnly) {
						linker.objs.push_back(objName);
					}
					
				}
				
				std::string mainOut;
				outputMainEntryPoint(OUT_INTERMEDIATE, mainOut);
				outputMainEntryPoint(OUT_OBJECT, mainOut);
				if (!this->options.compileOnly) {
					linker.objs.push_back(mainOut);
				}
				
			}
			else {
				throw Compiler::Error(*this, "No executable section");
			}


			if (!this->options.compileOnly) {
				linker.outputFileName = executableCodePtr->name;

				linker.link(*this);
			}

			if (executableCodePtr) {
				executableCodePtr.reset();
			}
		}

		void Compiler::compile(const language::AST& ast)
		{
			stage1(ast);

			createMainEntryPoint();
			
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

		void Compiler::stage1(const AST& ast)
		{ 
		
			Stage1 s1(*this);
			ast.visitAll(s1);

			if (!executableCodePtr) {
				programInst = new Program();
				programInst->name = "a";
				executableCodePtr.reset(programInst);
				Module* m = new Module(*this);
				executableCodePtr->modules.insert(std::make_pair(m->name, m));
			}

		}

		void Compiler::stage1VisitNode(const language::ParseNode& p)
		{

		}

		void Compiler::clearEnvironment()
		{
			globalEnv.clear();
			classes.clear();
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
			
			switch (outFmt) {
				case OUT_INTERMEDIATE: {
					outputFileName = name + ".ll";
				}
				break;

				case OUT_OBJECT: {
					outputFileName = name + ".o";
				}
				break;
			}
			
			c.outputModule(outFmt, *modulePtr, outputFileName);
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
			
			std::system(link_command.c_str());
		}
	}
	
}
