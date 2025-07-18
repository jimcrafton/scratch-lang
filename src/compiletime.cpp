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

	Module::Module(compiler::Compiler& c):
		compiler(c)
	{
		init(compiler);
	}

	Module::~Module()
	{
		
	}

	void Module::init(compiler::Compiler& c)
	{
		name = "a";

		llvmModulePtr.reset(c.createModule(name));

		globalEnv = new ScopedEnvironment(c);
		globalEnv->name = "module scope " + name;

		c.getGlobalScope()->add(globalEnv);
	}


	void Module::resetScope()
	{
		if (nullptr == globalEnv) {
			globalEnv = new ScopedEnvironment(compiler);
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


	bool ScopedEnvironment::addType(const TypeDecl& t)
	{
		auto name = t.fullyQualifiedName();
		if (hasType(name)) {
			return false;
		}
		auto res = types.insert(std::make_pair(name, t));
		res.first->second.setScope(this);
		return true;
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

	bool ScopedEnvironment::addVariable(const Variable& v)
	{
		if (hasVariable(v.getName())) {
			//throw Compiler::Error(compiler, "Variable already exists");
			return false;
		}

		printf("'%s' : '%s'\n", v.getName().c_str(), v.type.getName().c_str());

		auto res = variables.insert(std::make_pair(v.getName(), v));
		res.first->second.setScope(this);

		return true;
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

	void ScopedEnvironment::addStringLiteral(const CppString& name, const datatypes::string& strlit)
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
		if (var.type.type != compiletime::TypeDescriptor::typeUnknown) {
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


	std::map<TypeDescriptor, std::string> Compiletime::primitiveTypes;

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

	bool Compiletime::init()
	{
		Compiletime::initBasicTypes();
		return true;
	}

	const std::map<TypeDescriptor, std::string>& Compiletime::globalPrims()
	{
		return Compiletime::primitiveTypes;
	}
}
