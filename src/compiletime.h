#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <atomic>


#include "AST.h"

#include <chrono>

#include "datatypes.h"


#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

/*
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
*/



namespace compiler {
	class Compiler;
}





namespace compiletime {
	typedef std::string CppString;


	typedef typesystem::TypeDescriptor TypeDescriptor;


	

	
	class Type {
	public:
			

		CppString typeNamespace;
		CppString name;
		TypeDescriptor type = TypeDescriptor::typeUnknown;

		Type() {}
		Type(const Type& rhs) :name(rhs.name), typeNamespace(rhs.typeNamespace), type(rhs.type) {}
		Type(const CppString& n, TypeDescriptor t) :name(n),type(t) {}
		Type(const CppString& n, const CppString& nn, TypeDescriptor t) :name(n), typeNamespace(nn), type(t) {}
		virtual ~Type() {}

		bool isPrimitive() const {
			return type != TypeDescriptor::typeRecord && type != TypeDescriptor::typeClass;
		}

			

		static const Type& null() {
			static Type result;

			return result;
		}


	};

	typedef std::string CppString;



	class Flag {
	public:
		CppString val;
	};

	typedef std::map<CppString, Flag> Flags;
	class ScopedEnvironment;
	class Namespace;
	class NamespaceBlock;
	class TypeDecl;
	class Variable;
	class Instance;
	class Message;
	class SendMessage;
	class Module;


	class Class;

	/*
	* common base class that links back to the AST object and into the actual source file
	*/
	class CodeElement {
	public:
		CodeElement() {}		

		CodeElement(const CppString& n) :name(n) {}

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

		const NamespaceBlock* getNamespace() const { return namspaceBlk; }
		void setNamespace(const NamespaceBlock* v) { namspaceBlk = v; }


		virtual CppString getFullyQualifiedName() const;

		virtual bool scopeConsideredGlobal() const { return false; }


		const Flags& getFlags() const { return flags; }
		Flags& getFlags() { return flags; }
		bool hasFlag(const CppString& flag) const { return flags.count(flag) != 0; }
		const Flag* getFlag(const CppString& flag) const { 
			if (flags.count(flag) == 0) {
				return nullptr;
			}
			const auto& found = flags.find(flag);
			return &found->second;
		}

	protected:
		Flags flags;
		CppString name;


		lexer::CodeLocation code;
		const ScopedEnvironment* scope = nullptr;
		const NamespaceBlock* namspaceBlk = nullptr;
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


		virtual bool scopeConsideredGlobal() const { return true; }
	private:

	};




	class TypeDecl : public CodeElement {
	public:


		Namespace typeNamespace;
		compiletime::TypeDescriptor type = compiletime::TypeDescriptor::typeUnknown;

		TypeDecl() {}
		TypeDecl(const TypeDecl& rhs) :CodeElement(rhs), typeNamespace(rhs.typeNamespace), type(rhs.type) {}
		TypeDecl(const CppString& n, compiletime::TypeDescriptor t) :CodeElement(n), type(t) {}
		TypeDecl(const CppString& n, const CppString& nn, compiletime::TypeDescriptor t) :CodeElement(n), typeNamespace(nn), type(t) {}
		virtual ~TypeDecl() {}

		bool isPrimitive() const {
			return type != compiletime::TypeDescriptor::typeRecord &&
				type != compiletime::TypeDescriptor::typeClass;
		}

		CppString fullyQualifiedName() const {
			CppString result = typeNamespace.fullyQualified(name);
			return result;
		}

		static const TypeDecl& null() {
			static TypeDecl result;

			return result;
		}


		llvm::Type* getType(::llvm::IRBuilder<> *builder ) const;

		virtual bool scopeConsideredGlobal() const { return true; }
	};




	class Primitive : public CodeElement {
	public:


		union Data {

			typesystem::boolT boolV;
			typesystem::int8T int8V;
			typesystem::uint8T uint8V;
			typesystem::int16T int16V;
			typesystem::uint16T uint16V;
			typesystem::int32T int32V;
			typesystem::uint32T uint32V;
			typesystem::int64T int64V;
			typesystem::uint64T uint64V;
			typesystem::int128T int128V;
			typesystem::uint128T uint128V;

			typesystem::float32T floatV;
			typesystem::double64T doubleV;
		};

		Data data;
		TypeDecl type;


		virtual bool scopeConsideredGlobal() const { return true; }
	};




	class Variable : public CodeElement {
	public:
		TypeDecl typeDecl;

		Namespace varNamespace;

		bool globalVar = false;

		virtual ~Variable() {}

		bool empty() const {
			return flags.empty() && name.empty() && typeDecl.type == compiletime::TypeDescriptor::typeUnknown && scope == nullptr;
		}
		static const Variable& null() {
			static Variable res;
			return res;
		}

		
	};

	class Instance : public CodeElement {
	public:

		

		union Value {
			const datatypes::object* objPtr;
			const datatypes::string* stringPtr;
			const Primitive* primitivePtr = nullptr;
		};

		TypeDecl typeDecl;
		Value value;

		const Variable* variable = nullptr;

		
	};


	class MessageParameter : public CodeElement {
	public:
		const Instance* param = nullptr;
		TypeDecl typeDecl;
		Instance defValue;
	};

	class MessageBlock;

	class MessageDecl : public CodeElement {
	public:

		virtual ~MessageDecl() {}
		
		std::vector<std::pair<CppString, MessageParameter>> parameters;

		TypeDecl returnType;

		CppString className;

		virtual CppString getFullyQualifiedName() const;

		CppString getNameWithArgs() const;

		MessageBlock* impl = nullptr;
	};



	class Message : public CodeElement {
	public:

		virtual ~Message() {}

		enum Type {
			msgTypeUnknown = 0,
			msgTypeMathOperator   = 1,
			msgTypeAsignmentOperator = 2,
			msgTypeCompareEquals,
			msgTypeComparison,
			msgTypeNamed
		};

		Type type;

		std::vector<std::pair<CppString, MessageParameter>> parameters;

		Instance* returnVal=nullptr;
	};


	class MessageSelector {
	public:
		MessageSelector() {}
		MessageSelector(const Message& m) :name(m.getName()), type(m.type){}

		CppString name;
		Message::Type type;
	};


	class SendMessage : public CodeElement {
	public:

		virtual ~SendMessage() {}


		bool async = false;

		Message msg;
		const Instance* instance = nullptr;
		

		MessageSelector getSelector() const {
			return MessageSelector(msg);
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


	class ClassBlock;

	class MessageBlock : public Block {
	public:


		PreConditionBlock preConditions;
		PostConditionBlock postConditions;

		MessageDecl* msgDecl = nullptr;
		const Instance* returnVal = nullptr;

		CppString className;
	};

	class ExtendsBlock : public Block {
	public:

		CppString extensionTarget;


		std::map<CppString, MessageDecl> messages;
		std::map<CppString, MessageBlock> implementations;
	};

	class ClassBlock : public Block {
	public:

		CppString superClassName;

		std::map<CppString, VariablesBlock> memberVars;

		std::map<CppString, MessageDecl> messages;
		
		std::map<CppString, MessageBlock> implementations;
	};


	class CodeFragmentBlock : public Block {
	public:
		VariablesBlock vars;

		std::unordered_map<CppString, ExtendsBlock> extendsDefs;
		std::unordered_map<CppString, ClassBlock> classDefs;
	};


	class NamespaceBlock : public Block {
	public:

		NamespaceBlock* parent;

		


		CodeFragmentBlock code;

		ScopedEnvironment* scope;

		std::unordered_map<CppString, NamespaceBlock> namespaces;

		virtual bool scopeConsideredGlobal() const { return true; }
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

		Module(compiler::Compiler& c);
		virtual ~Module();

		CppString version;


		std::unique_ptr<::llvm::Module> llvmModulePtr;

		void init(compiler::Compiler& c);

		//void output(compiler::Compiler& c, OutputFormat outFmt, std::string& outputFileName);

		const std::map<CppString, Import>& getImports() const { return imports; }

		const ScopedEnvironment* globalScope() const { return globalEnv; }
		ScopedEnvironment* globalScope() { return globalEnv; }

		const ScopedEnvironment* getCurrentScope() const { return currentScope; }
		ScopedEnvironment* getCurrentScope() { return currentScope; }

		void resetScope();

		void closeScope(const ScopedEnvironment& scope);
		void closeCurrentScope();
		ScopedEnvironment* popCurrentScope();
		ScopedEnvironment* createNewScope(const CppString& name, CodeElement* scopeElement);
		ScopedEnvironment* pushNewScope(const CppString& name, CodeElement* scopeElement);

		void createGlobalScope(compiler::Compiler& c);


		const NamespaceBlock* getCurrentNamespace() const { return currentNamespace; }
		NamespaceBlock* getCurrentNamespace() { return currentNamespace; }
		void closeCurrentNamespace();
		NamespaceBlock* popCurrentNamespace();
		NamespaceBlock* createNewNamespace(const CppString& name);


		const NamespaceBlock* getGlobalNamespace() const { return &globalNamespace; }
		NamespaceBlock* getGlobalNamespace() { return &globalNamespace; }

		virtual bool scopeConsideredGlobal() const { return true; }

		ExtendsBlock* getCurrentExtendsBlock() { return currentExtendsBlock; }
		const ExtendsBlock* getCurrentExtendsBlock() const { return currentExtendsBlock; }
		void setCurrentExtendsBlock(ExtendsBlock* val) { currentExtendsBlock =val; }

		ClassBlock* getCurrentClassBlock() { return currentClassBlock; }
		const ClassBlock* getCurrentClassBlock() const { return currentClassBlock; }
		void setCurrentClassBlock(ClassBlock* val) { currentClassBlock = val; }

		MessageBlock* getCurrentMessageBlock() { return currentMessageBlock; }
		const MessageBlock* getCurrentMessageBlock() const { return currentMessageBlock; }
		void setCurrentMessageBlock(MessageBlock* val) { currentMessageBlock = val; }
	private:
		ScopedEnvironment* globalEnv = nullptr;
		ScopedEnvironment* currentScope = nullptr;

		compiler::Compiler& compiler;

		std::map<CppString, Import> imports;
		NamespaceBlock globalNamespace;
		NamespaceBlock* currentNamespace;

		ExtendsBlock* currentExtendsBlock;
		ClassBlock* currentClassBlock;
		MessageBlock* currentMessageBlock;
		
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

		CppString version = "1.0.0";

		ModuleMapT modules;


		virtual void init(const CppString& n, compiler::Compiler& c);

		Module* primaryModule();

		virtual int findStartup() const { return 0; }

		virtual const MessageDecl* getStartupMain() const { return nullptr; }
	};

	class Program : public ExecutableFragment {
	public:
		CppString mainEntryFunction = "main:";
		virtual ~Program() {}

		virtual int findStartup() const ;
		virtual const MessageDecl* getStartupMain() const;
	};

	class Library : public ExecutableFragment {
	public:
		virtual ~Library() {}
		CppString libInitFunction = "libInit";
		CppString libTerminationFunction = "libTerm";

		virtual int findStartup() const ;
		virtual const MessageDecl* getStartupMain() const;
	};




	class ScopedEnvironment {
	public:

		


		ScopedEnvironment(const compiler::Compiler& c) :compiler(c), parent(nullptr) {}

		~ScopedEnvironment() {
			clear();
		}

		ScopedEnvironment* parent = nullptr;

		std::vector<ScopedEnvironment*> children;

		CppString name;


		Module* module = nullptr;

		CodeElement* scopedElement = nullptr;

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

		bool hasType(const CppString& name, bool recursive = false) const {
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
		const TypeDecl& getType(const CppString& name, bool recursive = false) const {
			if (recursive) {
				bool res = types.count(name) != 0;
				if (!res) {
					if (nullptr != parent) {
						return parent->getType(name, true);
					}
				}
			}
			if (types.count(name) == 0) {
				return TypeDecl::null();
			}
			return types.find(name)->second;
		}

		TypeDecl* addType(const TypeDecl& t);

		bool hasInstance(const CppString& name) const {
			return instances.count(name) != 0;
		}

		bool hasVariable(const CppString& name) const {
			return variables.count(name) != 0;
		}

		const Instance& getInstance(const CppString& name) const {
			return instances.find(name)->second;
		}

		const Variable& getVariable(const CppString& name, bool recursive = false) const {

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

		const std::map<CppString, TypeDecl>& getTypes() const {
			return types;
		}

		const std::map<CppString, Instance>& getInstances() const {
			return instances;
		}

		const std::map<CppString, Variable>& getVariables() const {
			return variables;
		}

		Instance* addInstance(const Instance& i);

		Variable* addVariable(const Variable& v);


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

		SendMessage* addSendMessage(const SendMessage& msg);

		void addStringLiteral(const CppString& name, const datatypes::string& strlit);

		bool hasStringLiteral(const CppString& name) const {
			return strings.count(name) != 0;
		}

		const datatypes::string& getStringLiteral(const CppString& name) const {
			return strings.find(name)->second;
		}


		Primitive* addPrimLiteral(const CppString& name, const Primitive& prim);

		bool hasPrimLiteral(const CppString& name) const {
			return primitives.count(name) != 0;
		}

		const Primitive& getPrimLiteral(const CppString& name) const {
			return primitives.find(name)->second;
		}

		bool resolveVariable(const Variable& var, Variable& outVar);

		const Instance* getSelf() const { return selfPtr; }
		void setSelf(const Instance* v) { selfPtr = v; }

		bool isGlobal() const {
			return module == scopedElement;
		}


		void addLocalVar(const CppString& name, llvm::AllocaInst* var) const ;
		llvm::AllocaInst* getLocalVar(const CppString& name) const;
	private:

		//this should be the object instance that the scope is part of 
		//for the method that is executed
		//if it's null, then it should default to the 'runtime' instance
		const Instance* selfPtr = nullptr;

		const compiler::Compiler& compiler;
		std::map<CppString, TypeDecl> types;
		std::map<CppString, Instance> instances;
		std::map<CppString, Variable> variables;
		std::map<CppString, SendMessage> msgSends;

		std::map<CppString, datatypes::string> strings;

		std::map<CppString, Primitive> primitives;

		mutable std::map<CppString, llvm::AllocaInst*> localVars;
	};



	//*****************************************************************************

	class ReturnValue {
	public:
		ReturnValue() {}
		ReturnValue(bool v) :success(v) {}

		operator bool () const {
			return success;
		}

		bool operator==(bool rhs) const { return success == rhs; }
		ReturnValue& operator=(bool rhs) { success = rhs; return *this;}

	private:
		bool success = false;
	};

	

	class InstanceMethod {
	public:
		typedef std::function < ReturnValue(const Instance*, const std::vector<const Instance*>&) > MethodT;

		InstanceMethod() {}
		InstanceMethod(const CppString& n, const CppString& ln, MethodT m, Class* c) :name(n), longname(ln), method(m), clazz(c){}

		CppString name;
		CppString longname;
		MethodT method;
		Class* clazz=nullptr;
	};

	class ClassMethod {
	public:
		typedef std::function < ReturnValue(const std::vector<const Instance*>&) > MethodT;
		ClassMethod() {}
		ClassMethod(const CppString& n, const CppString& ln, MethodT m, Class* c) :name(n), longname(ln), method(m), clazz(c) {}

		CppString name;
		CppString longname;
		MethodT method;
		Class* clazz = nullptr;
	};


	class Field {
	public:
		CppString name;
		compiletime::Type type;
	};

	class Record {
	public:
		std::unordered_map<CppString, Field> instanceVars;
		std::unordered_map<CppString, Field> classVars;
	};

	class Class : public Record {
	public:
		CppString name;
		Class* super;

		ReturnValue sendMessage(const CppString& selector, const Instance* receiver, const std::vector<const Instance*>& params);

		void addInstanceMethod(const CppString& name, const CppString& longname, InstanceMethod::MethodT m) {
			instanceMethods[name] = InstanceMethod(name, longname, m, this);
		}

		void addClassMethod(const CppString& name, const CppString& longname, ClassMethod::MethodT m) {
			classMethods[name] = ClassMethod(name, longname, m, this);
		}

		std::unordered_map<CppString, InstanceMethod> instanceMethods;
		std::unordered_map<CppString, ClassMethod> classMethods;


		const InstanceMethod* getInstanceMethod(const CppString& selector, bool longform) const {
			if (longform) {
				for (auto m: instanceMethods) {
					if (m.second.longname == selector) {
						return &m.second;
					}
				}
			}
			else {
				auto found = instanceMethods.find(selector);
				if (found != instanceMethods.end()) {
					return &found->second;
				}
			}
			
			return nullptr;
		}

		const ClassMethod* getClassMethod(const CppString& selector, bool longform) const {
			if (longform) {
				for (const auto& m : classMethods) {
					if (m.second.longname == selector) {
						return &m.second;
					}
				}
			}
			else {
				auto found = classMethods.find(selector);
				if (found != classMethods.end()) {
					return &found->second;
				}
			}
			
			return nullptr;
		}

		bool hasInstanceMethod(const CppString& selector, bool longform) const {
			if (longform) {
				for (auto m : instanceMethods) {
					if (m.second.longname == selector) {
						return true;
					}
				}
				return false;
			}
			return instanceMethods.count(selector) == 0 ? false : true;			
		}

		bool hasClassMethod(const CppString& selector, bool longform) const {
			if (longform) {
				for (auto m : classMethods) {
					if (m.second.longname == selector) {
						return true;
					}
				}
				return false;
			}
			return classMethods.count(selector) == 0 ? false : true;
		}

		static InstanceMethod::MethodT NoOpInstance;
		static ClassMethod::MethodT NoOpClass;

	};

	typedef std::map<CppString, Class*> ClassMap;
	class Compiletime {
	public:


		class Error {
		public:
			std::string message;		

			Error(const std::string& err) :message(err) {}

		};


		class MessageNotFound: public Error {
		public:
			MessageNotFound(const std::string& err) :Error(err) {}

		};

		class SelectorEmpty : public Error {
		public:
			SelectorEmpty(const std::string& err) :Error(err) {}

		};


		class ClassAlreadExists : public Error {
		public:
			ClassAlreadExists(const std::string& err) :Error(err) {}
		};

		class IncorrectSuperClass : public Error {
		public:
			IncorrectSuperClass(const std::string& err) :Error(err) {}
		};


		static bool init();

		static void terminate();

		static const std::map< TypeDescriptor, std::string>& globalPrims();

		static Class* getClass(const CppString& name);

		static void addClass(Class* clazz, Class* superClazz=nullptr);
	private:			
		Compiletime();
		
		static ClassMap classes;

		static std::map<TypeDescriptor, std::string> primitiveTypes;
		static void initBasicTypes();
	};
}  //namespace compiletime
