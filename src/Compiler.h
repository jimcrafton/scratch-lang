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

#include "AST.h"





#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
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

namespace language {

	namespace compiler {

		typedef std::string CppString;
		 
		class Compiler;


		enum OutputFormat {
			OUT_INTERMEDIATE = 1,
			OUT_OBJECT,
		};

		class Primitive {
		public:
			enum Type {
				typeBit1,
				typeInteger8=8,
				typeUInteger8=9,
				typeInteger16=16,
				typeUInteger16=17,
				typeShort = typeInteger16,
				typeUShort = typeUInteger16,
				typeInteger32=32,
				typeUInteger32=33,
				typeInteger64=64,
				typeUInteger64=65,
				typeInteger128 = 128,
				typeUInteger128 = 129,

				typeDouble32 = 34,
				typeFloat = typeDouble32,
				typeDouble64 = 66,
				typeDouble = typeDouble64,

			};

			union Data{
				struct Type128{
					int64_t lo;
					int64_t hi;
				};

				struct TypeU128 {
					uint64_t lo;
					uint64_t hi;
				};

				bool boolV;
				int8_t intV;
				uint8_t uintV;
				int16_t int16V;
				uint16_t uint16V;
				int32_t int32V;
				uint32_t uint32V;
				int64_t int64V;
				uint64_t uint64V;				
				Type128 int128V;
				TypeU128 uint128V;

				float floatV;
				double doubleV;
			};

			Data data;
		};

		class Object {

		};




		template<typename T>
		class list {
		public:

		};

		template<typename T>
		class array : public list<T> {
		public:
			std::vector<T> data;
		};

		template<typename T>
		class dictionary : public list<T> {
		public:
			std::map<CppString,T> data;
		};

		class string {

		};



		class Instance {
		public:
			
			
			enum Type {
				instPrimitive=0,
				instObject
			};

			union Value {
				Object* objPtr;
				Primitive* primitivePtr;
			};

			CppString name;
			Type type;
			Value value;

		};






		class Flag {
		public:
			CppString val;
		};

		typedef dictionary<Flag> Flags;

		class MessageParameter {
		public:
			CppString name;
			Flags flags;

			Instance defValue;
		};


		class Message {
		public:
			CppString name;
			Flags flags;

			std::unordered_map<CppString, MessageParameter> parameters;
		};


		class Block {
		public:
			Flags flags;
		};

		class VariablesBlock : public Block {

		};


		class PreConditionBlock : public Block {

		};

		class PostConditionBlock : public Block {

		};

		
		

		class MessageBlock : public Block {
		public:		
			CppString name;

			PreConditionBlock preConditions;
			PostConditionBlock postConditions;

			

			//CodeFragmentBlock code;
		};

		

		class ClassBlock : public Block {
		public:
			CppString name;
			CppString superClassName;

			std::map<CppString, VariablesBlock> memberVars;

			std::map<CppString, MessageBlock> messages;
		};


		class CodeFragmentBlock : public Block {
		public:
			VariablesBlock vars;
			CppString name;

			std::unordered_map<CppString, ClassBlock> classDefs;
		};


		class NamespaceBlock : public Block {
		public:
			CppString name;
			CppString parent;

			

			CodeFragmentBlock code;
		};

		class ThreadBlock : public Block {

		};

		class Import {
		public:
			CppString name;
			CppString localPath;
			CppString remotePath;
		};

		class Module {
		public:
			
			Module(Compiler& c) { init(c); }

			CppString name = "a";
			std::map<CppString, Import> imports;
			std::map<CppString, NamespaceBlock> namespaces;

			std::unique_ptr<llvm::Module> modulePtr;

			void init(Compiler& c);

			void output(Compiler& c, OutputFormat outFmt, std::string& outputFileName);
		};

		

		class ExecutableFragment {
		public:
			virtual ~ExecutableFragment() {
				for (auto m : modules) {
					delete m.second;
				}
				modules.clear();
			}

			typedef std::map<CppString, Module*> ModuleMapT;
			CppString name="a";
			CppString version="1.0.0";

			ModuleMapT modules;
		};

		class Program : public ExecutableFragment {
		public:
			CppString mainEntryFunction;
			virtual ~Program() {}
		};

		class Library : public ExecutableFragment {
		public:
			virtual ~Library() {}
			CppString libInitFunction;
			CppString libTerminationFunction;
		};





		class Class {
		public:
			CppString name;

			std::map<CppString, Message > message;

		};


		class Stage1;


		class ScopedEnvironment {
		public:
			ScopedEnvironment() {}

			~ScopedEnvironment() {
				clear();
			}

			ScopedEnvironment* parent = nullptr;

			std::vector<ScopedEnvironment*> children;

			void add(ScopedEnvironment* scope) {
				children.push_back(scope);
			}

			void clear() {
				for (auto child : children) {
					delete child;
				}

				children.clear();
			}
		};


		enum CompilerErrorType {
			UNKNOWN_ERR = 0,
		};

		class CompilerOptions {
		public:
			bool compileOnly = false;
			void init(utils::cmd_line_options& cmdline);
		};


		class Compiler {
		public:
			class Error {
			public:
				const Compiler& compiler;
				std::string message;
				
				CompilerErrorType errCode = UNKNOWN_ERR;

				Error(Compiler& c, const std::string& err) :compiler(c), message(err){}


				void output() const {
					std::cout << "Compiler error: " << message << std::endl;
				}
			};

			Compiler() {
				init();
			}

			~Compiler() {
				terminate();
			}

			void build(const std::vector<std::string>& files);

			//build up initial structures like
			//modules, program, etc
			//if no module is found then create one called "a"
			//and since no module was found there won't be a 
			//program/librarey either so assume program is 
			//called "a" as well
			void stage1(const language::AST& ast);


			void compile(const language::AST& ast);

			void dumpCompilerOutput();

			void dumpCompilerObjectCode();

			llvm::Module* createModule(const std::string& name);

			static void println(const std::string& msg);
			static void print(const std::string& msg, bool emitEndln=false);

			const llvm::TargetMachine* getTargetMachine() const { return llvmTargetMachinePtr.get(); }
			llvm::TargetMachine* getTargetMachine() { return llvmTargetMachinePtr.get(); }

			void outputModule(OutputFormat outFmt, llvm::Module&, const std::string& outfileName);

			CompilerOptions options;
		private:
			friend class Stage1;

			

			
			std::unique_ptr<llvm::LLVMContext> llvmCtxPtr;
			std::unique_ptr<llvm::Module> mainEntryModulePtr;
			std::unique_ptr<llvm::IRBuilder<>> llvmBuilderPtr;
			std::unique_ptr<llvm::TargetMachine> llvmTargetMachinePtr;
			
			const llvm::Target* llvmTarget = nullptr;
			llvm::Triple llvmTargetTriple;

			std::unique_ptr <ExecutableFragment> executableCodePtr;
			Program* programInst = NULL;
			Program* libInst = NULL;

			ScopedEnvironment globalEnv;

			llvm::Function* mainEntryFunc = nullptr;


			std::map< CppString, Class> classes;

			void init();

			void terminate();

			void stage1VisitNode(const language::ParseNode& p);

			void clearEnvironment();

			void createMainEntryPoint();

			void outputMainEntryPoint(OutputFormat outFmt, std::string& outfileName);
		};



		class Linker {
		public:
			Linker() { init(); }

			~Linker() { terminate(); }

			void link(const Compiler& compiler);

			std::string outputFileName;

			std::vector<std::string> objs;
		private:
			void init();

			void terminate();
		};

	}

}