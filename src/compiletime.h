#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <atomic>


#include "types.h"


#include "AST.h"

#include <chrono>





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


	namespace compiletime {
		typedef std::string CppString;


		typedef language::typesystem::TypeDescriptor TypeDescriptor;


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
}  //language