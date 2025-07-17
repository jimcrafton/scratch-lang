#include "compiletime.h"



namespace language {
	namespace compiletime {
		

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
}