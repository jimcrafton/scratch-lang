#include "compiletime.h"

#include "Compiler.h"


namespace compiletime {
		
	/*

	llvm::Value* Instance::getValue(llvm::IRBuilder<>* builder) const
	{
		llvm::Value* result = nullptr;

		switch (type.type) {
		case compiletime::TypeDescriptor::typeBool: {
			result = value.primitivePtr->data.boolV ? builder->getTrue() : builder->getFalse();
		}break;

		case compiletime::TypeDescriptor::typeInteger8: {
			result = builder->getInt8(value.primitivePtr->data.int8V);
		}break;

		case compiletime::TypeDescriptor::typeUInteger8: {

			result = builder->getInt8(value.primitivePtr->data.uint8V);
		}break;

		case compiletime::TypeDescriptor::typeUInteger32: {

			result = builder->getInt32(value.primitivePtr->data.uint32V);
		}break;

		case compiletime::TypeDescriptor::typeInteger32: {

			result = builder->getInt32(value.primitivePtr->data.int32V);
		}break;

		case compiletime::TypeDescriptor::typeString: {

		}break;

		default: {
			result = nullptr;
		}break;
		}

		return result;
	}
	*/


	CppString Primitive::toString() const
	{
		CppString result;

		

		switch (type.type) {
			case compiletime::TypeDescriptor::typeInteger8: {
				result = std::to_string(this->data.int8V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger8: {
				result = std::to_string(this->data.uint8V);
			}break;

			case compiletime::TypeDescriptor::typeInteger16: {
				result = std::to_string(this->data.int16V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger16: {
				result = std::to_string(this->data.uint16V);
			}break;

			case compiletime::TypeDescriptor::typeInteger32: {
				result = std::to_string(this->data.int32V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger32: {
				result = std::to_string(this->data.uint32V);
			}break;

			case compiletime::TypeDescriptor::typeInteger64: {
				result = std::to_string(this->data.int64V);
			}break;

			case compiletime::TypeDescriptor::typeUInteger64: {
				result = std::to_string(this->data.uint64V);
			}break;

			case compiletime::TypeDescriptor::typeDouble32: {
				result = std::to_string(this->data.floatV);
			}break;

			case compiletime::TypeDescriptor::typeDouble64: {
				result = std::to_string(this->data.doubleV);
			}break;			

			default: {
				result = "unkown instance type??";
			}break;
		}

		return result;
	}

	CppString CodeElement::getFullyQualifiedName() const
	{
		CppString result = name;
		
		auto ns = getNamespace();
		while (ns != nullptr) {
			if (!ns->getName().empty()) {
				result = ns->getName() + Namespace::Separator + result;
			}

			ns = ns->parent;
		}

		return result;
	}

	llvm::Type* TypeDecl::getType(::llvm::IRBuilder<>* builder) const
	{
		llvm::Type* result = nullptr;
		switch (type) {
			case compiletime::TypeDescriptor::typeBool: {
				result = builder->getInt1Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger8: 
			case compiletime::TypeDescriptor::typeUInteger8: {
				result = builder->getInt8Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger16: 
			case compiletime::TypeDescriptor::typeUInteger16: {
				result = builder->getInt16Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger32:
			case compiletime::TypeDescriptor::typeUInteger32: {
				result = builder->getInt32Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger64:
			case compiletime::TypeDescriptor::typeUInteger64: {
				result = builder->getInt64Ty();
			}break;

			case compiletime::TypeDescriptor::typeInteger128:
			case compiletime::TypeDescriptor::typeUInteger128: {
				result = builder->getInt128Ty();
			}break;

			case compiletime::TypeDescriptor::typeDouble32: {
				result = builder->getFloatTy();
			}break;

			case compiletime::TypeDescriptor::typeDouble64: {
				result = builder->getDoubleTy();
			}break;

			case compiletime::TypeDescriptor::typeString: {
				result = builder->getPtrTy();

			}break;


			default: {
				result = nullptr;
			}break;
		}

		return result;
	}

	Module::Module(compiler::Compiler& c):
		compiler(c)
	{
		name = "a";
	}

	Module::~Module()
	{
		
	}

	void Module::init(compiler::Compiler& c)
	{
		llvmModulePtr.reset(c.createModule(getName()));

		createGlobalScope(c);

		currentNamespace = &globalNamespace;
	}

	void Module::createGlobalScope(compiler::Compiler& c)
	{
		globalEnv = new ScopedEnvironment(c);
		globalEnv->name = "module scope " + name;
		globalEnv->module = this;
		globalEnv->scopedElement = this;

		c.getGlobalScope()->add(globalEnv);

		globalNamespace.setScope(globalEnv);

		resetScope();
	}

	void Module::resetScope()
	{
		if (nullptr == globalEnv) {
			throw std::runtime_error("No global env!");
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


	void Module::closeCurrentNamespace()
	{
		popCurrentNamespace();
	}

	NamespaceBlock* Module::popCurrentNamespace()
	{
		currentNamespace = currentNamespace->parent;
		if (nullptr == currentNamespace) {
			currentNamespace = &globalNamespace;
		}

		return currentNamespace;
	}

	NamespaceBlock* Module::createNewNamespace(const CppString& name)
	{
		NamespaceBlock newNamespace;
		newNamespace.setName(name);
		auto res = currentNamespace->namespaces.insert(std::make_pair(name, newNamespace));
		auto& ns = res.first->second;
		ns.parent = currentNamespace;
		currentNamespace = &ns;

		return currentNamespace;
	}


	void ExecutableFragment::init(const CppString& n, compiler::Compiler& c)
	{
		name = n;
		Module* m = new Module(c);
		m->setName(name);
		m->init(c);
		modules.insert(std::make_pair(m->getName(), m));
		m->resetScope();
	}

	Module* ExecutableFragment::primaryModule()
	{
		if (modules.empty()) {
			return nullptr;
		}
		auto first = modules.begin();
		return first->second;
	}


	int Program::findStartup() const 
	{
		int result = 0;
		for (auto m : modules) {
			auto& module = *m.second;
			for (const auto& extension : module.getGlobalNamespace()->code.extendsDefs) {
				for (const auto& impl : extension.second.implementations) {
					if (impl.second.msgDecl->getNameWithArgs() == mainEntryFunction) {
						result++;
					}
				}
			}
		}

		return result;
	}

	const MessageDecl* Program::getStartupMain() const
	{
		const MessageDecl* result = nullptr;

		for (auto m : modules) {
			auto& module = *m.second;
			//find main
			// find the class that represents this module

			for (const auto& extension : module.getGlobalNamespace()->code.extendsDefs) {
				for (const auto& impl: extension.second.implementations) {
					if (impl.second.msgDecl->getNameWithArgs() == mainEntryFunction) {
						result = impl.second.msgDecl;
					}
				}
			}
		}

		return result;
	}

	int Library::findStartup() const
	{
		//library must have and init and term function defined!

		int result = 0;
		for (auto m : modules) {
			auto& module = *m.second;
			//find main
			// find the class that represents this module
			compiletime::Class* clazz = compiletime::Compiletime::getClass(module.getName());
			if (nullptr != clazz) {
				if (clazz->hasClassMethod(libInitFunction,true)) {
					if (clazz->hasClassMethod(libTerminationFunction,true)) {
						result++;
					}
				}
			}
		}


		return result;
	}

	const MessageDecl* Library::getStartupMain() const
	{
		const MessageDecl* result = nullptr;

		for (auto m : modules) {
			auto& module = *m.second;
			//find main

			for (const auto& extension : module.getGlobalNamespace()->code.extendsDefs) {
				bool initFound = false;
				bool termFound = false;
				for (const auto& impl : extension.second.implementations) {
					if (impl.second.msgDecl->getNameWithArgs() == libInitFunction) {
						//result = impl.second.msgDecl;
						initFound = true;
					}
					if (impl.second.msgDecl->getNameWithArgs() == libTerminationFunction) {
						//result = impl.second.msgDecl;
						termFound = true;
					}

					if (initFound && termFound) {
						result = impl.second.msgDecl;
						break;
					}
				}
			}
		}


		return result;
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

	ScopedEnvironment* Module::createNewScope(const CppString& name, CodeElement* scopeElement) {
		ScopedEnvironment* result = new ScopedEnvironment(compiler);
		result->module = this;
		result->scopedElement = scopeElement;
		result->name = name;
		currentScope->add(result);
		return result;
	}

	ScopedEnvironment* Module::pushNewScope(const CppString& name, CodeElement* scopeElement) {
		ScopedEnvironment* newScope = createNewScope(name,scopeElement);
		currentScope = newScope;
		return currentScope;
	}


	TypeDecl* ScopedEnvironment::addType(const TypeDecl& t)
	{
		auto name = t.fullyQualifiedName();
		if (hasType(name)) {
			return nullptr;
		}
		auto res = types.insert(std::make_pair(name, t));
		res.first->second.setScope(this);
		return &res.first->second;
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
			std::cout << tab << "\t" << i.second.getName() << " " << i.second.typeDecl.type << std::endl;
		}
		std::cout << line << std::endl;
		std::cout << tab << "\tvariables" << std::endl;
		for (const auto& v : getVariables()) {
			std::cout << tab << "\t" << v.second.getName() << " '" << v.second.typeDecl.fullyQualifiedName() << "' : " << v.second.typeDecl.type << std::endl;
		}
		std::cout << line << std::endl;
		std::cout << tab << "\tsend messages" << std::endl;
		for (const auto& msg : getMsgSends()) {
			std::cout << tab << "\t" << msg.first /* << " " << v.second.type.fullyQualifiedName() << ":" << v.second.type.type*/ << std::endl;
		}
	}

	Instance* ScopedEnvironment::addInstance(const Instance& i)
	{
		if (hasInstance(i.getName())) {
			//throw Compiler::Error(compiler, "Instance already exists");
			return nullptr;
		}

		auto res = instances.insert(std::make_pair(i.getName(), i));
		res.first->second.setScope(this);
		return &res.first->second;
	}

	Variable* ScopedEnvironment::addVariable(const Variable& v)
	{
		if (hasVariable(v.getName())) {
			//throw Compiler::Error(compiler, "Variable already exists");
			return nullptr;
		}

		printf("'%s' : '%s'\n", v.getName().c_str(), v.typeDecl.getName().c_str());

		auto res = variables.insert(std::make_pair(v.getName(), v));
		res.first->second.setScope(this);
		res.first->second.setNamespace(this->module->getCurrentNamespace());

		return &res.first->second;
	}

	SendMessage* ScopedEnvironment::addSendMessage(const SendMessage& msg)
	{
		std::string key = msg.msg.getName() + "_to_" + (msg.instance != nullptr ? msg.instance->getName() : std::string("null"));
		if (!msg.msg.parameters.empty()) {
			key += "_with_";
			for (auto p : msg.msg.parameters) {

			}
		}
		msgSends.push_back(std::make_pair(key, msg));
		auto& last = msgSends.back();
		last.second.setScope(this);

		return &last.second;
	}

	void ScopedEnvironment::addStringLiteral(const CppString& name, const datatypes::string& strlit)
	{
		if (strings.count(name) != 0) {
			return;
		}
		auto res = strings.insert(std::make_pair(name, strlit));
	}

	Primitive* ScopedEnvironment::addPrimLiteral(const CppString& name, const Primitive& prim)
	{
		if (primitives.count(name) != 0) {
			return nullptr;
		}
		auto res = primitives.insert(std::make_pair(name, prim));
		return &res.first->second;
	}

	bool ScopedEnvironment::resolveVariable(const Variable& var, Variable& outVar)
	{
		Variable* foundVar = nullptr;
		if (var.typeDecl.type != compiletime::TypeDescriptor::typeUnknown) {
			return true;
		}

		auto res = hasType(var.typeDecl.getName(), true);
		printf("%s (%s) has type %d\n", var.getName().c_str(), var.typeDecl.getName().c_str(), (int)res);
		if (res) {
			auto t = getType(var.typeDecl.getName(), true);
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
				foundVar->typeDecl = t;
				outVar = *foundVar;
			}
		}
		return foundVar == nullptr ? false : true;
	}


	void ScopedEnvironment::addLocalVar(const CppString& name, llvm::AllocaInst* var) const
	{
		if (localVars.count(name) == 0) {
			localVars.insert(std::make_pair(name, var));
		}
	}

	llvm::AllocaInst* ScopedEnvironment::getLocalVar(const CppString& name) const
	{
		llvm::AllocaInst* result = nullptr;
		auto found = localVars.find(name);
		if (found != localVars.end()) {
			result = found->second;
		}
		return result;
	}

	std::map<TypeDescriptor, std::string> Compiletime::primitiveTypes;
	ClassMap Compiletime::classes;

	void Compiletime::initBasicTypes()
	{
		primitiveTypes = {
			{TypeDescriptor::typeInteger8, "int8"},
			{TypeDescriptor::typeUInteger8, "uint8"},
			{TypeDescriptor::typeInteger16, "int16"},
			{TypeDescriptor::typeUInteger16, "uint16"},
			{TypeDescriptor::typeInteger32, "int32"},
			{TypeDescriptor::typeUInteger32, "uint32"},
			{TypeDescriptor::typeInteger64, "int64"},
			{TypeDescriptor::typeUInteger64, "uint64"},
			{TypeDescriptor::typeInteger128, "int128"},
			{TypeDescriptor::typeUInteger128, "uint128"},
			{TypeDescriptor::typeDouble32, "real32"},
			{TypeDescriptor::typeDouble64, "real64"},
			{TypeDescriptor::typeBool, "bool"},
			{TypeDescriptor::typeString, "string"},
			{TypeDescriptor::typeArray, "array"},
			{TypeDescriptor::typeDictionary, "dictionary"},
			{TypeDescriptor::typeMessage, "message"}
		};
	}




	InstanceMethod::MethodT Class::NoOpInstance = [](const Instance* self, const std::vector<const Instance*>& params) {

		return compiletime::ReturnValue(true);
		};


	ClassMethod::MethodT Class::NoOpClass = [](const std::vector<const compiletime::Instance*>& params) {

		return compiletime::ReturnValue(true);
		};



	ReturnValue Class::sendMessage(const CppString& selector, const Instance* receiver, const std::vector<const Instance*>& params)
	{
		bool foundMethod = false;

		if (selector.empty()) {
			std::string msg = "message selector is empty for ";
			if (nullptr == receiver) {
				msg += "class '" + name + "'";
			}
			else {
				msg += "reciever instance '" + receiver->getName() + "' class '" + name + "'";
			}

			throw Compiletime::SelectorEmpty(msg);
		}

		if (nullptr != receiver) {
			auto found = instanceMethods.find(selector);
			if (found == instanceMethods.end()) {
				//try super
				if (nullptr != super) {
					return super->sendMessage(selector, receiver, params);
				}
			}
			else {
				auto& instanceMethod = found->second;
				foundMethod = true;
				return instanceMethod.method(receiver, params);
			}
		}
		else {
			auto found = classMethods.find(selector);
			if (found == classMethods.end()) {
				//try super
				if (nullptr != super) {
					return super->sendMessage(selector, nullptr, params);
				}
			}
			else {
				foundMethod = true;
				auto& classMethod = found->second;				
				return classMethod.method(params);
			}
		}

		if (!foundMethod) {
			std::string msg = "Unable to find message selector '" + selector + "' for " ;
			if (nullptr == receiver) {
				msg += "class '" + name + "'";
			}
			else {
				msg += "reciever instance '" + receiver->getName() + "' class '" + name +"'";
			}

			throw Compiletime::MessageNotFound(msg);
		}

		return ReturnValue(false); //indicates unable to find a handler for the message
	}

	bool Compiletime::init()
	{
		Compiletime::initBasicTypes();

		
		for (auto type: primitiveTypes) {
			Class* primClass = nullptr;

			if (isTypeInt(type.first)) {
				primClass = new IntClass(type.first);
			}
			else if (isTypeRealNumber(type.first)) {
				primClass = new RealClass(type.first);
			}
			else {
				primClass = new Class();
				primClass->name = type.second;
			}		

			Compiletime::classes.insert( std::make_pair(primClass->name, primClass)  );
		}


		Class* moduleClass = new Class();
		moduleClass->name = "module";
		Compiletime::classes.insert(std::make_pair(moduleClass->name, moduleClass));

		


		return true;
	}

	void Compiletime::terminate()
	{
		for (auto clazz : Compiletime::classes) {
			delete clazz.second;
		}
		Compiletime::classes.clear();
	}



	Class* Compiletime::getClass(const CppString& name)
	{
		auto found = Compiletime::classes.find(name);
		if (found!= Compiletime::classes.end()) {
			return found->second;
		}
		return nullptr;
	}

	void Compiletime::addClass(Class* clazz, Class* superClazz)
	{
		if (Compiletime::classes.count(clazz->name) != 0) {
			auto msg = "Class '" + clazz->name + "' already exists";
			throw Compiletime::ClassAlreadExists(msg);
		}


		if (clazz->super != nullptr) {
			if (clazz->super != superClazz && (nullptr != superClazz)) {
				auto msg = "Class '" + clazz->name + "' already has a super class and doesn't match super class '" + superClazz->name + "'";
				throw Compiletime::IncorrectSuperClass(msg);
			}
		}

		clazz->super = superClazz;

		Compiletime::classes.insert(std::make_pair(clazz->name, clazz));
	}

	std::string Compiletime::primName(const TypeDescriptor& td)
	{
		std::string result;
		if (Compiletime::primitiveTypes.count(td)) {
			auto found = Compiletime::primitiveTypes.find(td);
			result = found->second;
		}
		return result;
	}

	const std::map<TypeDescriptor, std::string>& Compiletime::globalPrims()
	{
		return Compiletime::primitiveTypes;
	}

	CppString MessageDecl::getNameWithArgs() const
	{
		CppString result;

		result = getName();

		if (parameters.size() == 1) {
			result += ":";
		}
		else if (parameters.size() > 1) {
			result += ":";
			for (auto p : parameters) {
				const auto& param = p.second;

				result += param.getName() + ":";
			}
		}

		return result;
	}

	CppString MessageDecl::getFullyQualifiedName() const
	{
		CppString result;

		auto nsName = getNamespace()->getFullyQualifiedName();
		auto moduleName = getScope()->module->getName();
		result = ((moduleName == className) ? "" : moduleName + ".") +
			(nsName.empty() ? "" : nsName + ".") +
			className + "." + getNameWithArgs();

		return result;
	}


	IntClass::IntClass(TypeDescriptor td) : Class()
	{
		isSigned = isTypeUnsigned(td);
		
		name = Compiletime::primName(td);

		switch (td) {
			case compiletime::TypeDescriptor::typeInteger8: case compiletime::TypeDescriptor::typeUInteger8: {
				width = Width::bits8;
			
			}break;
			case compiletime::TypeDescriptor::typeInteger16: case compiletime::TypeDescriptor::typeUInteger16: {
				width = Width::bits16;
			}break;

			case compiletime::TypeDescriptor::typeInteger32: case compiletime::TypeDescriptor::typeUInteger32: {
				width = Width::bits32;
			}break;

			case compiletime::TypeDescriptor::typeInteger64: case compiletime::TypeDescriptor::typeUInteger64: {
				width = Width::bits64;
			}break;

			case compiletime::TypeDescriptor::typeInteger128: case compiletime::TypeDescriptor::typeUInteger128: {
				width = Width::bits128;
			}break;
			default: {
				width = Width::bitsUnknown;
			} break;
		}
	}


	RealClass::RealClass(TypeDescriptor td)
	{
		name = Compiletime::primName(td);
		switch (td) {

			case compiletime::TypeDescriptor::typeDouble32: {
				width = Width::bits32;
			}break;

			case compiletime::TypeDescriptor::typeDouble64:{
				width = Width::bits64;
			}break;

			default: {
				width = Width::bitsUnknown;
			} break;
		}
	}


}
