#pragma once


#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include "types.h"

namespace language {

	namespace runtime {
		class Runtime;

		namespace datatypes {



			class character {
			public:
				typedef char charT;
			};

			class string {
			public:

				typedef character::charT charT;

				string();
				~string();

				string(const string& rhs);
				string& operator=(const string& rhs);


				uint64_t hash() const;

				void assign(const std::string& v);

				void assign(const charT* c_strPtr);

				uint64_t length() const;
				uint64_t empty() const;
			private:
				void* data=nullptr;
			};




			class objectinfo {
			public:				

				static constexpr uint64_t NilObjectInfo = (uint64_t) - 1;
				//size in bytes for the object. for stack based 
				//primitive this would be the storage required for
				//the object, i.e. int32T would be 4, 
				//uint8T wiould be 1, etc
				uint64_t size = NilObjectInfo;
				language::typesystem::TypeDescriptor type = language::typesystem::TypeDescriptor::typeUnknown;

				inline bool isNil() const {
					auto n = objectinfo::nil();
					return size == n.size && type == n.type;
				}

				static const objectinfo& nil();
			};

			class objectinfo_flags {
			public:
				enum FlagVals{
					flagMemOnStack = 0,
					flagMemOnHeap = 1
				};


				objectinfo_flags(): memtype(flagMemOnStack), unused1(0), unused2(0), unused3(0){}

				inline void setMemType(bool onHeap) { memtype = onHeap ? flagMemOnHeap : flagMemOnStack; }
				inline bool isMemOnHeap() const { return memtype== flagMemOnHeap ? true:false; }
				inline bool isMemOnStack() const { return memtype == flagMemOnStack ? true : false; }

				inline operator uint32_t () const { return  (uint32_t)(*((uint32_t*)this)); }

				uint8_t memtype : 1; //flagMemOnStack or flagMemOnHeap
				uint8_t unused1 : 7;
				uint8_t unused2 : 8;
				uint16_t unused3 : 16;
			};

			class objectinfo_ptr {
			public:
				static constexpr uint32_t NoIndx = (uint32_t)-1;

				uint32_t infoIdx = NoIndx;
				objectinfo_flags flags;
			};
			/*
			some sort of placeholder around runtime memory,
			can be a number, actual object, etc. 
			can differentiate between low level primitives like
			number (int32, or floating point), bools, 
			or actual objects
			should be able to distinguish between local/stakc storage
			vs heap storage
			*/
			class object {
			public:
				const objectinfo& info() const;

			
			private:
				friend class language::runtime::Runtime;

				objectinfo_ptr infoptr;

			};

			class list : public object {
			public:

			};


			class array : public object {
			public:

			};


			class dictionary : public object {
			public:

			};




			class Record : public object {
			public:
				
			};

			class Class : public Record {
			public:
				
			};
			
		}

		
		class StringPool;

		class Runtime {
		public:

			enum MainReturnValues {
				mainOK = 0,
				mainGeneralException = -1,
				mainNoMainEntryPointSpecified,
				mainRuntimeInitFailed,
				mainRuntimeFinishFailed,
			};


			typedef int (*MainEntryPointFuncT)(Runtime* self, const std::vector<std::string>& args);



			class Error {
			public:
				MainReturnValues errorCode = MainReturnValues::mainGeneralException;

				std::string message;

				Error(MainReturnValues err, const std::string& msg):errorCode(err),message(msg) {}
			};



			bool init();
			bool finish();


			static int mainCrtEntryPoint(int argc, char** argv);

			static int mainRtEntryPoint(const std::vector<std::string>& args);


			MainEntryPointFuncT main = nullptr;


			static void registerType(language::typesystem::TypeDescriptor type);
			static void registerClass(const datatypes::string& name);
			static void registerRecord(const datatypes::string& name);

			static const language::runtime::datatypes::objectinfo& getInfo(const language::runtime::datatypes::object& obj);

			static const language::runtime::datatypes::Record* getRecord(const language::runtime::datatypes::object& obj);
			static const language::runtime::datatypes::Class* getClass(const language::runtime::datatypes::object& obj);
		private:
			friend class StringPool;

			Runtime();
			~Runtime();

			StringPool* stringPool = nullptr;

			std::vector< language::runtime::datatypes::objectinfo>  objInfoList;

			std::vector< language::runtime::datatypes::Record*>  recordInfoList;
			std::vector< language::runtime::datatypes::Class*>  classInfoList;
			

			static Runtime rtInstance;
		};
	}
}