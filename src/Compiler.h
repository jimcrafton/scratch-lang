#pragma once



/*
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
*/




#include <vector>
#include <map>
#include <unordered_map>
#include <string>



namespace language {

	namespace compiler {

		typedef std::string CppString;
		 

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


		class Message {
		public:
			CppString name;
			Flags flags;
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

		class CodeFragmentBlock : public Block {
		public:
			VariablesBlock vars;
			CppString name;
		};

		
		class MessageParameter {
		public:
			CppString name;
			Flags flags;

			Instance defValue;
		};

		class MessageBlock : public Block {
		public:		
			CppString name;

			PreConditionBlock preConditions;
			PostConditionBlock postConditions;

			std::unordered_map<CppString, MessageParameter> parameters;

			CodeFragmentBlock code;
		};

		

		class ClassBlock : public Block {
		public:
			CppString name;
			CppString superClassName;

			std::map<CppString, VariablesBlock> memberVars;

			std::map<CppString, MessageBlock> messages;
		};


		class NamespaceBlock : public Block {
		public:
			CppString name;
			CppString parent;

			std::unordered_map<CppString, ClassBlock> classDefs;
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
			std::map<CppString, Import> imports;
			std::map<CppString, NamespaceBlock> namespaces;
		};

		

		class ExecutableFragment {
		public:
			CppString name;
			CppString version;

			std::map<CppString, Module> modules;
		};

		class Program : public ExecutableFragment {
		public:
			CppString mainEntryFunction;
		};

		class Library : public ExecutableFragment {
			CppString libInitFunction;
			CppString libTerminationFunction;
		};





		class Class {
		public:
			CppString name;

			std::map<CppString, Message > message;

		};




		class Compiler {
		public:
			static void init();
			static void finish();




		private:
			/*
			static std::unique_ptr<llvm::LLVMContext> llvmCtxPtr;
			static std::unique_ptr<llvm::Module> modulePtr;
			static std::unique_ptr<llvm::IRBuilder<>> llvmBuilderPtr;
			*/

			Program* programInst = NULL;
			Program* libInst = NULL;

			std::map< CppString, Class> classes;


			Compiler() {

			}

			~Compiler() {

			}

			static Compiler* compilerInstance;
		};



	}

}