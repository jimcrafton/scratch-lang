#pragma once



/*
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"

*/









#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <atomic>
#include <chrono>



#include "AST.h"
#include "compiletime.h"
#include "runtime.h"




#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/TargetParser/SubtargetFeature.h"
#include "llvm/ADT/StringMap.h"


namespace utils {
	class cmd_line_options;
}


namespace parser {
	class ModuleBlock;
	class ClassBlock;
	class RecordBlock;
	class VariableNode;
	class InstanceNode;
	class MessageDeclaration;
	class MessageBlock;
	class SendMessage;
}





namespace compiler {


	class CompilerOptions {
	public:

		static constexpr auto COMP_OPT_HELP = "help";
		static constexpr auto COMP_OPT_VERSION = "version";
		static constexpr auto COMP_OPT_PRINT_AST = "print-ast";
			
		static constexpr auto COMP_OPT_NO_LOGO = "no-logo";
		static constexpr auto COMP_OPT_VERBOSE_MODE = "verbose-mode";
		static constexpr auto COMP_OPT_DEBUG_MODE = "debug-mode";
		static constexpr auto COMP_OPT_COMPILE_ONLY = "compile-only";
			
			

		bool compileOnly = false;
		bool noLogo = false;
		bool verboseMode = false;
		bool debugMode = false;
		bool printAST = false;
			
		void init(const utils::cmd_line_options& cmdline);
	};


		
		 
	class Compiler;


	enum OutputFormat {
		OUT_INTERMEDIATE = 1,
		OUT_OBJECT,
	};
								


	class Stage1;
	class ModuleCheck;


	typedef compiletime::CppString CppString;


	enum CompilerErrorType {
		UNKNOWN_ERR = 0,
		NO_VARIABLE_TYPE_NAME,
		NO_VARIABLE_TYPE_DEFINED,
	};

		


	class Compiler {
	public:
		class Error {
		public:
			const Compiler& compiler;
			std::string message;
				
			CompilerErrorType errCode = UNKNOWN_ERR;

			Error(const Compiler& c, const std::string& err) :compiler(c), message(err){}


			void output() const;
		};
			
		class ErrorItem {
		public:

			std::string message;
			lexer::CodeLocation code;				
			CompilerErrorType errCode = UNKNOWN_ERR;
			const compiletime::CodeElement* element = nullptr;
		private:
				
		};


		Compiler(const utils::cmd_line_options&);

		~Compiler();


		void build(const std::vector<std::string>& files);

		//build up initial structures like
		//modules, program, etc
		//if no module is found then create one called "a"
		//and since no module was found there won't be a 
		//program/librarey either so assume program is 
		//called "a" as well
		void stage1(const language::AST& ast);

		void stage2();

		void stage3();

		void compileAST(const language::AST& ast);

		void compile();

		void link();


		::llvm::Module* createModule(const std::string& name);

		static void println(const std::string& msg);
		static void print(const std::string& msg, bool emitEndln=false);

		const ::llvm::TargetMachine* getTargetMachine() const { return llvmTargetMachinePtr.get(); }
		::llvm::TargetMachine* getTargetMachine() { return llvmTargetMachinePtr.get(); }

		void outputModule(OutputFormat outFmt, ::llvm::Module&, const std::string& outfileName);

		CompilerOptions options;

		static std::string version();
		static std::string logo();

		static std::string mainEntryFunction();

		void outputErrors() const;

		const compiletime::ScopedEnvironment* getGlobalScope() const { return globalEnv; }
		compiletime::ScopedEnvironment* getGlobalScope() { return globalEnv; }
	private:
		friend class Stage1;
		friend class  ModuleCheck;
		const utils::cmd_line_options& cmdlineOpts;
			
		void stage1NewModule(const parser::ModuleBlock& node);
		void stage1NewClass(const parser::ClassBlock& node);
		void stage1CloseClassBlock(const parser::ClassBlock& node);
			
		void stage1NewMessageBlock(const parser::MessageBlock& node);
		void stage1CloseMessageBlock(const parser::MessageBlock& node);
		void stage1NewMessageDecl(const parser::MessageDeclaration& node);
		void stage1NewSendMessage(const parser::SendMessage& node);
		void stage1NewAssignmentMessage(const parser::Assignment& node);
		void stage1NewRecord(const parser::RecordBlock& node);
		void stage1CloseRecord(const parser::RecordBlock& node);
		void stage1Variable(const parser::VariableNode& node);
		void stage1Instance(const parser::InstanceNode& node);
		void stage1Instance(const parser::MsgInstanceNode& node);

		void stage1Literal(const parser::LiteralNode& node);
		void stage1Nil(const parser::NilNode& node);
			
			
		std::string name;
		std::unique_ptr<::llvm::LLVMContext> llvmCtxPtr;
		std::unique_ptr<::llvm::Module> mainEntryModulePtr;
		std::unique_ptr<::llvm::IRBuilder<>> llvmBuilderPtr;
		std::unique_ptr<::llvm::TargetMachine> llvmTargetMachinePtr;
			
		const ::llvm::Target* llvmTarget = nullptr;
		::llvm::Triple llvmTargetTriple;

		std::unique_ptr <compiletime::ExecutableFragment> executableCodePtr;
		compiletime::Program* programInst = NULL;
		compiletime::Program* libInst = NULL;

			
		compiletime::Module* currentModule = nullptr;
		compiletime::ScopedEnvironment* globalEnv = nullptr;

		::llvm::Function* mainEntryFunc = nullptr;


		std::map< CppString, compiletime::Class> classes;
		std::vector<ErrorItem> errorListing;


		std::chrono::system_clock::time_point buildStart;
		std::chrono::system_clock::time_point buildEnd;


		void init();
		void initBasicTypes();

		void terminate();

		void clearEnvironment();

		void createMainEntryPoint();

		void outputMainEntryPoint(OutputFormat outFmt, std::string& outfileName);

		void printBuildStats();

		std::string generateTempInstanceName(const language::ParseNode* node);

		void buildFunctions(const compiletime::ScopedEnvironment& scope);
		void buildFunction(const compiletime::Instance* receiver, const compiletime::Message& msg, const compiletime::ScopedEnvironment& scope);
		void buildFunction(const compiletime::Instance* receiver, const CppString& selector, const std::vector<const compiletime::Instance*>& params, const compiletime::ScopedEnvironment& scope);

		void outputModule(compiletime::Module* module, OutputFormat outFmt, const CppString& objName);
	};


	class LinkerOptions {
	public:			
		bool noLogo = false;
		bool verboseMode = false;
		bool debugMode = false;			
		void init(const utils::cmd_line_options& cmdline);
	};

	class Linker {
	public:
		Linker(const utils::cmd_line_options& opts) { 
			options.init(opts);
			init(); 
		}

		~Linker() { terminate(); }

		void link(const Compiler& compiler);

		std::string outputFileName;

		std::vector<std::string> objs;
	private:
		LinkerOptions options;

		void init();

		void terminate();

		void reportSuccessfulOutput();
	};

}
