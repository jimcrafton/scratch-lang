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

namespace language {

	namespace compiler {

		class CompilerOptions {
		public:
			bool compileOnly = false;
			bool noLogo = false;
			bool verboseMode = false;
			bool debugMode = false;
			bool printAST = false;
			void init(const utils::cmd_line_options& cmdline);
		};


		typedef std::string CppString;
		 
		class Compiler;


		enum OutputFormat {
			OUT_INTERMEDIATE = 1,
			OUT_OBJECT,
		};

		class Flag {
		public:
			CppString val;
		};

		


		class Type {
		public:
			enum TypeOf {
				typeUnknown,
				typeRecord,
				typeClass,
				typeBit1,
				typeInteger8		= 8,
				typeUInteger8		= 9,
				typeInteger16		= 16,
				typeUInteger16		= 17,
				typeShort			= typeInteger16,
				typeUShort			= typeUInteger16,
				typeInteger32		= 32,
				typeUInteger32		= 33,
				typeInteger64		= 64,
				typeUInteger64		= 65,
				typeInteger128		= 128,
				typeUInteger128		= 129,

				typeDouble32		= 34,
				typeFloat			= typeDouble32,
				typeDouble64		= 66,
				typeDouble			= typeDouble64,
				typeBool = 100,
				typeString,
				typeArray,
				typeDictionary,
			};

			CppString name;
			CppString namespaceName;
			TypeOf type = Type::typeUnknown;

			Type() {}
			Type(const CppString& n, TypeOf t) :name(n), type(t) {}
			Type(const CppString& n, const CppString& nn, TypeOf t) :name(n), namespaceName(nn),type(t) {}

			bool isPrimitive() const {
				return type == typeRecord || type == typeClass;
			}

			CppString fullyQualifiedName() const {
				CppString result = "";
				if (!namespaceName.empty()) {
					result = namespaceName + ".";
				}
				result += name;
				return result;
			}

			static const Type& null() {
				static Type result;

				return result;
			}
		};


		class Primitive {
		public:
			

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

		class Record {

		};

		class Object : Record {

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

		typedef dictionary<Flag> Flags;

		




		class ScopedEnvironment;

		class Variable {
		public:
			Flags flags;
			CppString name;
			Type type;
			ScopedEnvironment* scope = nullptr;
		};

		class Instance {
		public:

			union Value {
				Object* objPtr;
				Primitive* primitivePtr;
			};

			CppString name;
			Type type;
			Value value;
			ScopedEnvironment* scope=nullptr;
		};

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


		class SendMessage {
		public:
			bool async = false;

			Message msg;
			const Instance* instance = nullptr;
			ScopedEnvironment* scope = nullptr;
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
			CppString version;
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
			ScopedEnvironment(const Compiler& c):compiler(c) {}

			~ScopedEnvironment() {
				clear();
			}

			ScopedEnvironment* parent = nullptr;

			std::vector<ScopedEnvironment*> children;

			

			void add(ScopedEnvironment* scope) {
				scope->parent = this;
				children.push_back(scope);
			}

			void clear() {
				for (auto child : children) {
					delete child;
				}

				children.clear();
			}

			bool hasType(const CppString& name, bool recursive=false) const {
				if (recursive) {
					bool res = types.count(name) != 0;
					if (!res) {						
						if (nullptr != parent) {
							return parent->hasType(name, true);
						}
					}
				}
				return types.count(name) != 0;
			}
			const Type& getType(const CppString& name, bool recursive = false) const {
				if (recursive) {
					bool res = types.count(name) != 0;
					if (!res) {
						if (nullptr != parent) {
							return parent->getType(name, true);
						}
					}
				}
				if (types.count(name) == 0) {
					return Type::null();
				}
				return types.find(name)->second;
			}

			void addType(const Type& t);

			bool hasInstance(const CppString& name) const {
				return instances.count(name) != 0;
			}

			bool hasVariable(const CppString& name) const {
				return variables.count(name) != 0;
			}

			const Instance& getInstance(const CppString& name) const {
				return instances.find(name)->second;
			}

			const Variable& getVariable(const CppString& name) const {
				return variables.find(name)->second;
			}

			const std::map<CppString, Type>& getTypes() const {
				return types;
			}

			const std::map<CppString, Instance>& getInstances() const {
				return instances;
			}

			const std::map<CppString, Variable>& getVariables() const {
				return variables;
			}

			void addInstance(const Instance& i);

			void addVariable(const Variable& v);

			void debugPrint() const {
				std::string tab = "";// "\t";
				auto p = parent;
				while (p) {
					p = p->parent;
					tab += "\t";
				}
				std::cout << tab << "\ttypes" << std::endl;
				for (const auto& t : getTypes()) {
					std::cout << tab << "\t" << t.second.name << " " << t.second.type << std::endl;
				}
				std::cout << tab << "\tinstances" << std::endl;
				for (const auto& i : getInstances()) {
					std::cout << tab << "\t" << i.second.name << " " << i.second.type.type << std::endl;
				}

				std::cout << tab << "\tvariables" << std::endl;
				for (const auto& v : getVariables()) {
					std::cout << tab << "\t" << v.second.name << " " << v.second.type.fullyQualifiedName() << ":" << v.second.type.type << std::endl;
				}

				std::cout << tab << "\tsend messages" << std::endl;
				for (const auto& msg : getMsgSends()) {
					std::cout << tab << "\t" << msg.first /* << " " << v.second.type.fullyQualifiedName() << ":" << v.second.type.type*/ << std::endl;
				}
			}

			bool hasSendMessage(const CppString& name) const {
				return msgSends.count(name) != 0;
			}

			const SendMessage& getSendMessage(const CppString& name) const {
				return msgSends.find(name)->second;
			}

			const std::map<CppString, SendMessage>& getMsgSends() const {
				return msgSends;
			}

			void addSendMessage(const SendMessage& msg);
		private:
			const Compiler& compiler;
			std::map<CppString, Type> types;
			std::map<CppString, Instance> instances;
			std::map<CppString, Variable> variables;
			std::map<CppString, SendMessage> msgSends;
			
			
			
		};


		enum CompilerErrorType {
			UNKNOWN_ERR = 0,
		};

		


		class Compiler {
		public:
			class Error {
			public:
				const Compiler& compiler;
				std::string message;
				
				CompilerErrorType errCode = UNKNOWN_ERR;

				Error(const Compiler& c, const std::string& err) :compiler(c), message(err){}


				void output() const {
					std::cout << "Compiler error: " << message << std::endl;
				}
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

			static std::string version();
			static std::string logo();

			
		private:
			friend class Stage1;
			const utils::cmd_line_options& cmdlineOpts;
			
			void stage1NewModule(const parser::ModuleBlock& node);
			void stage1NewClass(const parser::ClassBlock& node);
			void stage1CloseClassBlock(const parser::ClassBlock& node);
			
			void stage1NewMessageBlock(const parser::MessageBlock& node);
			void stage1CloseMessageBlock(const parser::MessageBlock& node);
			void stage1NewMessageDecl(const parser::MessageDeclaration& node);
			void stage1NewSendMessage(const parser::SendMessage& node);
			void stage1NewRecord(const parser::RecordBlock& node);
			void stage1Variable(const parser::VariableNode& node);
			void stage1Instance(const parser::InstanceNode& node);
			
			std::string name;
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
			ScopedEnvironment* currentScope = nullptr;

			llvm::Function* mainEntryFunc = nullptr;


			std::map< CppString, Class> classes;

			void init();
			void initBasicTypes();

			void terminate();

			void clearEnvironment();

			void createMainEntryPoint();

			void outputMainEntryPoint(OutputFormat outFmt, std::string& outfileName);

			ScopedEnvironment* createNewScope();
			ScopedEnvironment* pushNewScope();
			ScopedEnvironment* popCurrentScope();
			void closeCurrentScope();
			void closeScope(const ScopedEnvironment& scope);
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