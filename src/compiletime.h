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

	class CodeVisitor {
	public:
		virtual ~CodeVisitor() {}

		virtual void visitNamespace(const Namespace&) {}
		virtual void visitType(const TypeDecl&) {}
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

		virtual void accept(CodeVisitor& v) const {}

		CppString getFullyQualifiedName() const;

		virtual bool scopeConsideredGlobal() const { return false; }
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


		virtual void accept(CodeVisitor& v) const {
			v.visitNamespace(*this);
		}

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

		virtual void accept(CodeVisitor& v) const {
			v.visitType(*this);
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


		virtual ~Variable() {}

		bool empty() const {
			return flags.empty() && name.empty() && typeDecl.type == compiletime::TypeDescriptor::typeUnknown && scope == nullptr;
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
			const datatypes::object* objPtr;
			const datatypes::string* stringPtr;
			const Primitive* primitivePtr = nullptr;
		};

		TypeDecl typeDecl;
		Value value;

		virtual void accept(CodeVisitor& v) const {
			v.visitInstance(*this);
		}


		//llvm::Value* getValue(llvm::IRBuilder<>* builder) const;
	};

	class MessageParameter : public CodeElement {
	public:
		const Instance* param = nullptr;

		Instance defValue;
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

		std::unordered_map<CppString, MessageParameter> parameters;

		virtual void accept(CodeVisitor& v) const {
			v.visitMessage(*this);
		}
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
		virtual void accept(CodeVisitor& v) const {
			v.visitSendMessage(*this);
		}

		MessageSelector getSelector() const {
			return MessageSelector(msg);
		}
	};

	class Block : public CodeElement {
	public:
		ScopedEnvironment* blockScope = nullptr;
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

		virtual bool scopeConsideredGlobal() const { return true; }
	private:
		ScopedEnvironment* globalEnv = nullptr;
		ScopedEnvironment* currentScope = nullptr;

		compiler::Compiler& compiler;

		std::map<CppString, Import> imports;
		NamespaceBlock globalNamespace;
		NamespaceBlock* currentNamespace;
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


		std::unordered_map<CppString, MessageSelector > message;
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

		bool addType(const TypeDecl& t);

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

		void addInstance(const Instance& i);

		bool addVariable(const Variable& v);


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

		void addStringLiteral(const CppString& name, const datatypes::string& strlit);

		bool hasStringLiteral(const CppString& name) const {
			return strings.count(name) != 0;
		}

		const datatypes::string& getStringLiteral(const CppString& name) const {
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

		const Instance* getSelf() const { return selfPtr; }
		void setSelf(const Instance* v) { selfPtr = v; }

		bool isGlobal() const {
			return module == scopedElement;
		}
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


	};



	//*****************************************************************************

	class Compiletime {
	public:
		static bool init();

		static const std::map< TypeDescriptor, std::string>& globalPrims();
	private:			
		Compiletime();

		static std::map<TypeDescriptor, std::string> primitiveTypes;
		static void initBasicTypes();
	};
}  //namespace compiletime
