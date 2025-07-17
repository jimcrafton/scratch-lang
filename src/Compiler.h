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

		typedef std::map<CppString,Flag> Flags;
		class ScopedEnvironment;
		class Namespace;
		class Type;
		class Variable;
		class Instance;
		class Message;
		class SendMessage;
		class Module;


		class CodeVisitor {
		public:
			virtual ~CodeVisitor() {}

			virtual void visitNamespace(const Namespace&) {}
			virtual void visitType(const Type&) {}
			virtual void visitVariable(const Variable&) {}
			virtual void visitInstance(const Instance&) {}
			virtual void visitMessage(const Message&) {}
			virtual void visitSendMessage(const SendMessage&) {}
		};

		/*
		* common base class that links back to the AST object and into the actual source file
		*/
		class CodeElement {
		public:
			CodeElement() {}
			CodeElement(const CppString& n) :name(n){}
			
			virtual ~CodeElement() {}


			void setLocation(const lexer::CodeLocation& rhs) {
				code = rhs;
			}

			const lexer::CodeLocation& location() const {
				return code;
			}

			inline void setName(const CppString& v) { name = v; }
			inline const CppString& getName() const { return name; }

			inline void setScope(const ScopedEnvironment* v) { scope = v; }
			inline const ScopedEnvironment* getScope() const { return scope; }


			virtual void accept(CodeVisitor& v) const {}
		protected:
			Flags flags;
			CppString name;


			lexer::CodeLocation code;
			const ScopedEnvironment* scope = nullptr;
		};

		
		class Namespace : public CodeElement {
		public:

			static constexpr char Separator = '.';

			Namespace() {}
			Namespace(const CppString& n) :CodeElement(n) {}
			virtual ~Namespace() {}

			Namespace& operator=(const CppString& v) {
				name = v;
				return *this;
			}

			Namespace& addComponent(const CppString& v) {
				if (!name.empty()) {
					name += Namespace::Separator;
				}
				name += v;
				return *this;
			}
			

			operator const CppString& () const {
				return name;
			}


			CppString fullyQualified(const CppString& component) const {
				if (!name.empty()) {
					return name + Namespace::Separator + component;
				}
				return component;
			}

			bool empty() const { return name.empty(); }


			virtual void accept(CodeVisitor& v) const {
				v.visitNamespace(*this);
			}
		private:
			
		};




		class Type : public CodeElement {
		public:
			

			Namespace typeNamespace;
			language::compiletime::TypeDescriptor type = language::compiletime::TypeDescriptor::typeUnknown;

			Type() {}
			Type(const Type& rhs):CodeElement(rhs), typeNamespace(rhs.typeNamespace), type(rhs.type){}
			Type(const CppString& n, language::compiletime::TypeDescriptor t) :CodeElement(n), type(t) {}
			Type(const CppString& n, const CppString& nn, language::compiletime::TypeDescriptor t) :CodeElement(n), typeNamespace(nn),type(t) {}
			virtual ~Type() {}

			bool isPrimitive() const {
				return type != language::compiletime::TypeDescriptor::typeRecord && 
					type != language::compiletime::TypeDescriptor::typeClass;
			}

			CppString fullyQualifiedName() const {
				CppString result = typeNamespace.fullyQualified(name);
				return result;
			}

			static const Type& null() {
				static Type result;

				return result;
			}

			virtual void accept(CodeVisitor& v) const {
				v.visitType(*this);
			}
		};


		

		class Primitive : public CodeElement {
		public:
			

			union Data{				

				language::typesystem::boolT boolV;
				language::typesystem::int8T int8V;
				language::typesystem::uint8T uint8V;
				language::typesystem::int16T int16V;
				language::typesystem::uint16T uint16V;
				language::typesystem::int32T int32V;
				language::typesystem::uint32T uint32V;
				language::typesystem::int64T int64V;
				language::typesystem::uint64T uint64V;
				language::typesystem::int128T int128V;
				language::typesystem::uint128T uint128V;

				language::typesystem::float32T floatV;
				language::typesystem::double64T doubleV;
			};

			Data data;
			Type type;
		};

		

		

		




		

		


		class Variable : public CodeElement {
		public:			
			Type type;
			
			virtual ~Variable() {}

			bool empty() const {
				return flags.empty() && name.empty() && type.type == language::compiletime::TypeDescriptor::typeUnknown && scope == nullptr;
			}
			static const Variable& null() {
				static Variable res;
				return res;
			}

			virtual void accept(CodeVisitor& v) const {
				v.visitVariable(*this);
			}
		};

		class Instance : public CodeElement {
		public:

			union Value {
				const language::runtime::datatypes::object* objPtr;
				const language::runtime::datatypes::string* stringPtr;
				const Primitive* primitivePtr;
			};

			Type type;
			Value value;

			virtual void accept(CodeVisitor& v) const {
				v.visitInstance(*this);
			}


			llvm::Value* getValue(llvm::IRBuilder<>* builder) const ;
		};

		class MessageParameter : public CodeElement {
		public:
			Instance* param = nullptr;

			Instance defValue;
		};


		class Message : public CodeElement {
		public:
			
			virtual ~Message() {}

			std::unordered_map<CppString, MessageParameter> parameters;

			virtual void accept(CodeVisitor& v) const {
				v.visitMessage(*this);
			}
		};


		class SendMessage : public CodeElement {
		public:

			virtual ~SendMessage() {}


			bool async = false;

			Message msg;
			const Instance* instance = nullptr;
			virtual void accept(CodeVisitor& v) const {
				v.visitSendMessage(*this);
			}
		};

		class Block : public CodeElement {
		public:
			
		};

		class VariablesBlock : public Block {

		};


		class PreConditionBlock : public Block {

		};

		class PostConditionBlock : public Block {

		};

		
		

		class MessageBlock : public Block {
		public:		
			

			PreConditionBlock preConditions;
			PostConditionBlock postConditions;

			

			//CodeFragmentBlock code;
		};

		

		class ClassBlock : public Block {
		public:
			
			CppString superClassName;

			std::map<CppString, VariablesBlock> memberVars;

			std::map<CppString, MessageBlock> messages;
		};


		class CodeFragmentBlock : public Block {
		public:
			VariablesBlock vars;
			

			std::unordered_map<CppString, ClassBlock> classDefs;
		};


		class NamespaceBlock : public Block {
		public:
			
			CppString parent;

			

			CodeFragmentBlock code;
		};

		class ThreadBlock : public Block {

		};

		class Import : public CodeElement {
		public:
			
			CppString localPath;
			CppString remotePath;
		};

		class Module : public CodeElement {
		public:
			
			Module(Compiler& c):compiler(c){ init(compiler); }
			virtual ~Module();

			CppString version;
			

			std::unique_ptr<llvm::Module> llvmModulePtr;

			void init(Compiler& c);

			void output(Compiler& c, OutputFormat outFmt, std::string& outputFileName);

			const std::map<CppString, Import>& getImports() const { return imports; }
			const std::map<CppString, NamespaceBlock>& getNamespaces() const { return namespaces; }

			
			const ScopedEnvironment* globalScope() const { return globalEnv; }
			ScopedEnvironment* globalScope() { return globalEnv; }
			
			const ScopedEnvironment* getCurrentScope() const { return currentScope; }
			ScopedEnvironment* getCurrentScope() { return currentScope; }

			void resetScope();

			void closeScope(const ScopedEnvironment& scope);
			void closeCurrentScope();
			ScopedEnvironment* popCurrentScope();
			ScopedEnvironment* createNewScope();
			ScopedEnvironment* pushNewScope();
		private:
			ScopedEnvironment* globalEnv = nullptr;
			ScopedEnvironment* currentScope = nullptr;
			Compiler& compiler;

			std::map<CppString, Import> imports;
			std::map<CppString, NamespaceBlock> namespaces;
		};

		

		class ExecutableFragment : public CodeElement {
		public:

			ExecutableFragment() {
				name = "a";
			}

			virtual ~ExecutableFragment() {
				for (auto m : modules) {
					delete m.second;
				}
				modules.clear();
			}

			typedef std::map<CppString, Module*> ModuleMapT;
			
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





		class Class : public CodeElement {
		public:
			

			std::map<CppString, Message > message;

		};


		class Stage1;
		class ModuleCheck;



		class ScopedEnvironment {
		public:
			ScopedEnvironment(const Compiler& c):compiler(c), parent(nullptr){}

			~ScopedEnvironment() {
				clear();
			}

			ScopedEnvironment* parent = nullptr;

			std::vector<ScopedEnvironment*> children;

			CppString name;
			

			Module* module=nullptr;


			void add(ScopedEnvironment* scope) {
				scope->parent = this;
				children.push_back(scope);
			}

			void clear() {
				name.clear();
				module = nullptr;

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

			const Variable& getVariable(const CppString& name, bool recursive=false) const {
				
				if (variables.count(name) == 0) {
					if (recursive) {
						if (nullptr != parent) {
							return parent->getVariable(name, true);
						}
					}
					return Variable::null();
				}
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


			CppString getFullName() const;

			void debugPrint() const; 

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

			void addStringLiteral(const CppString& name, const language::runtime::datatypes::string& strlit);

			bool hasStringLiteral(const CppString& name) const {
				return strings.count(name) != 0;
			}

			const language::runtime::datatypes::string& getStringLiteral(const CppString& name) const {
				return strings.find(name)->second;
			}


			void addPrimLiteral(const CppString& name, const Primitive& prim);

			bool hasPrimLiteral(const CppString& name) const {
				return primitives.count(name) != 0;
			}

			const Primitive& getPrimLiteral(const CppString& name) const {
				return primitives.find(name)->second;
			}

			bool resolveVariable(const Variable& var, Variable& outVar);
		private:
			const Compiler& compiler;
			std::map<CppString, Type> types;
			std::map<CppString, Instance> instances;
			std::map<CppString, Variable> variables;
			std::map<CppString, SendMessage> msgSends;
			
			std::map<CppString, language::runtime::datatypes::string> strings;

			std::map<CppString, Primitive> primitives;
			
			
		};


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
				const CodeElement* element = nullptr;
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


			llvm::Module* createModule(const std::string& name);

			static void println(const std::string& msg);
			static void print(const std::string& msg, bool emitEndln=false);

			const llvm::TargetMachine* getTargetMachine() const { return llvmTargetMachinePtr.get(); }
			llvm::TargetMachine* getTargetMachine() { return llvmTargetMachinePtr.get(); }

			void outputModule(OutputFormat outFmt, llvm::Module&, const std::string& outfileName);

			CompilerOptions options;

			static std::string version();
			static std::string logo();

			static std::string mainEntryFunction();

			void outputErrors() const;

			const ScopedEnvironment* getGlobalScope() const { return &globalEnv; }
			ScopedEnvironment* getGlobalScope() { return &globalEnv; }
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
			std::unique_ptr<llvm::LLVMContext> llvmCtxPtr;
			std::unique_ptr<llvm::Module> mainEntryModulePtr;
			std::unique_ptr<llvm::IRBuilder<>> llvmBuilderPtr;
			std::unique_ptr<llvm::TargetMachine> llvmTargetMachinePtr;
			
			const llvm::Target* llvmTarget = nullptr;
			llvm::Triple llvmTargetTriple;

			std::unique_ptr <ExecutableFragment> executableCodePtr;
			Program* programInst = NULL;
			Program* libInst = NULL;

			
			Module* currentModule = nullptr;
			ScopedEnvironment globalEnv;

			llvm::Function* mainEntryFunc = nullptr;


			std::map< CppString, Class> classes;
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

			void buildFunctions(const ScopedEnvironment& scope);
			void buildFunction(const Instance* receiver, const Message& msg, const ScopedEnvironment& scope);
			void buildFunction(const Instance* receiver, const CppString& selector, const std::vector<const Instance*>& params, const ScopedEnvironment& scope);
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

}