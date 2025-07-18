#pragma once


#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include "types.h"
#include "datatypes.h"

namespace datatypes {
	class string_storage;
}

namespace runtime {
	class Runtime;

		

		

	class StringPool {
	public:
		std::unordered_map<uint64_t, datatypes::string_storage*> stringMap;

		static datatypes::string_storage* allocate();

		static datatypes::string_storage* allocate(const datatypes::string::charT* c_strPtr, size_t len);

		static void deallocate(datatypes::string_storage* str);
	};




	class Runtime {
	public:

		enum MainReturnValues {
			mainOK = 0,
			mainGeneralException = -1,
			mainNoMainEntryPointSpecified,
			mainRuntimeInitFailed,
			mainRuntimeFinishFailed,
		};


		typedef int (*MainEntryPointFuncT)(Runtime* self, const ::std::vector<::std::string>& args);



		class Error {
		public:
			MainReturnValues errorCode = MainReturnValues::mainGeneralException;

			::std::string message;

			Error(MainReturnValues err, const ::std::string& msg):errorCode(err),message(msg) {}
		};



		bool init();
		bool finish();


		static int mainCrtEntryPoint(int argc, char** argv);

		static int mainRtEntryPoint(const ::std::vector<::std::string>& args);


		MainEntryPointFuncT main = nullptr;


		static void registerType(typesystem::TypeDescriptor type);
		static void registerClass(const datatypes::string& name);
		static void registerRecord(const datatypes::string& name);

		static const datatypes::objectinfo& getInfo(const datatypes::object& obj);

		static const datatypes::Record* getRecord(const datatypes::object& obj);
		static const datatypes::Class* getClass(const datatypes::object& obj);
	private:
		friend class StringPool;

		Runtime();
		~Runtime();

		StringPool* stringPool = nullptr;

		::std::vector< datatypes::objectinfo>  objInfoList;

		::std::vector< datatypes::Record*>  recordInfoList;
		::std::vector< datatypes::Class*>  classInfoList;
			

		static Runtime rtInstance;
	};
}
