

#include "Runtime.h"
#include <unordered_map>


namespace language {

	namespace runtime {
		namespace datatypes {
			class string_storage {
			public:
				std::string data;
				uint64_t refcount = 0;
				uint64_t size = 0;
				StringPool* pool = nullptr;
				mutable uint64_t hashval = 0;

				string_storage() {}

				~string_storage() {

				}

				uint64_t hash_impl() const {
					if (0 == hashval) {
						hashval = std::hash<std::string>{}(data);
					}

					return hashval;
				}

				static uint64_t hash_impl(const datatypes::string::charT* c_strPtr, size_t len)  {
					return std::hash<std::string>{}(std::string(c_strPtr, len));
				}
			};
		}

		class StringPool {

		public:


			std::unordered_map<uint64_t, datatypes::string_storage*> stringMap;

			static datatypes::string_storage* allocate() {
				datatypes::string_storage* result = nullptr;

				result = new datatypes::string_storage();

				return result;
			}

			static datatypes::string_storage* allocate(const datatypes::string::charT* c_strPtr, size_t len) {
				datatypes::string_storage* result = nullptr;

				auto hashV = datatypes::string_storage::hash_impl(c_strPtr, len);
				
				result = new datatypes::string_storage();
				result->pool = Runtime::rtInstance.stringPool;
				result->hashval = hashV;
				result->data.assign(c_strPtr, len);

				if (nullptr != result->pool) {
					result->pool->stringMap.insert(std::pair<uint64_t, datatypes::string_storage*>(hashV, result));
				}

				return result;
			}

			static void deallocate(datatypes::string_storage* str) {
				printf("deallocating string instance %p... \n", str);
			}
		};



		namespace datatypes {
			

			string::string() 
			{
				data = language::runtime::StringPool::allocate();
			}

			string::~string()
			{
				string_storage * ptr = (string_storage*)data;

				language::runtime::StringPool::deallocate(ptr);
			}

			string::string(const string& rhs):
				data(rhs.data)
			{

			}

			string& string::operator=(const string& rhs)
			{
				data = rhs.data;

				return *this;
			}

			void string::assign(const std::string& v)
			{
				string_storage* ptr = (string_storage*)data;
				if (nullptr != ptr) {
					delete ptr;
				}
				data = language::runtime::StringPool::allocate(v.c_str(), v.length());
			}

			void string::assign(const charT* c_strPtr)
			{
				string_storage* ptr = (string_storage*)data;
				if (nullptr != ptr) {
					delete ptr;
				}

				data = language::runtime::StringPool::allocate(c_strPtr, strlen(c_strPtr));
			}

			uint64_t string::hash() const
			{
				string_storage* ptr = (string_storage*)data;
				
				return ptr->hash_impl();
			}

			uint64_t string::length() const
			{
				auto* ptr = (string_storage*)data;
				return ptr->data.length();
			}

			uint64_t string::empty() const 
			{
				auto* ptr = (string_storage*)data;
				return ptr->data.empty();
			}

			const objectinfo& objectinfo::nil() {
				static objectinfo result;
				return result;
			}

			const objectinfo& object::info() const
			{
				return language::runtime::Runtime::getInfo(*this);
			}


		}


		


		Runtime Runtime::rtInstance;
		Runtime::Runtime()
		{

		}

		Runtime::~Runtime()
		{

		}

		bool Runtime::init()
		{
			Runtime::rtInstance.stringPool = new StringPool();

			return true;
		}

		bool Runtime::finish()
		{
			delete Runtime::rtInstance.stringPool;
			Runtime::rtInstance.stringPool = nullptr;
			return true;
		}
	


		int Runtime::mainCrtEntryPoint(int argc, char** argv)
		{
			std::vector<std::string> args;

			for (int i = 0; i < argc;i++) {

				args.push_back( std::string(argv[i]) );
			}

			return Runtime::mainRtEntryPoint(args);
		}


		int Runtime::mainRtEntryPoint(const std::vector<std::string>& args)
		{ 
			MainReturnValues result = mainOK;

			try {
				if (Runtime::rtInstance.init()) {
					if (nullptr != Runtime::rtInstance.main) {
						int res = (*Runtime::rtInstance.main)(&Runtime::rtInstance, args);
						result = (MainReturnValues)res;
					}
					else {
						throw Runtime::Error(MainReturnValues::mainNoMainEntryPointSpecified, "Runtime main function is null");
					}

					if (!Runtime::rtInstance.finish()) {
						result = MainReturnValues::mainRuntimeFinishFailed;
					}
				}
				else {
					result = MainReturnValues::mainRuntimeInitFailed;
				}				
			}
			catch (const Runtime::Error& e) {
				result = e.errorCode;
			}
			catch (...) {
				result = MainReturnValues::mainGeneralException;
			}
		
			return (int)result;
		}


		const language::runtime::datatypes::objectinfo& Runtime::getInfo(const language::runtime::datatypes::object& obj)
		{
			if (obj.infoptr.infoIdx < Runtime::rtInstance.objInfoList.size()) {
				return Runtime::rtInstance.objInfoList[obj.infoptr.infoIdx];
			}
			

			return language::runtime::datatypes::objectinfo::nil();
		}


		const language::runtime::datatypes::Record* Runtime::getRecord(const language::runtime::datatypes::object& obj)
		{
			const language::runtime::datatypes::Record* result = nullptr;

			if (obj.infoptr.infoIdx < Runtime::rtInstance.recordInfoList.size()) {
				result = Runtime::rtInstance.recordInfoList[obj.infoptr.infoIdx];
			}
			

			return result; 
		}

		const language::runtime::datatypes::Class* Runtime::getClass(const language::runtime::datatypes::object& obj)
		{
			const language::runtime::datatypes::Class* result = nullptr;

			if (obj.infoptr.infoIdx < Runtime::rtInstance.classInfoList.size()) {
				result = Runtime::rtInstance.classInfoList[obj.infoptr.infoIdx];
			}


			return result;
		}




		


		void Runtime::registerClass(const datatypes::string& name)
		{

		}

		void Runtime::registerRecord(const datatypes::string& name)
		{
			
		}

		void Runtime::registerType(language::typesystem::TypeDescriptor type)
		{
			language::runtime::datatypes::objectinfo info;
			info.type = type;
			switch (info.type) {
				case language::typesystem::TypeDescriptor::typeBool :{
					info.size = 1;
				} break;

				case language::typesystem::TypeDescriptor::typeInteger8: {
					info.size = 1;
				} break;

				case language::typesystem::TypeDescriptor::typeUInteger8: {
					info.size = 1;
				} break;

				case language::typesystem::TypeDescriptor::typeInteger16: {
					info.size = 2;
				} break;

				case language::typesystem::TypeDescriptor::typeUInteger16: {
					info.size = 2;
				} break;

				case language::typesystem::TypeDescriptor::typeInteger32: {
					info.size = 4;
				} break;

				case language::typesystem::TypeDescriptor::typeUInteger32: {
					info.size = 4;
				} break;

				case language::typesystem::TypeDescriptor::typeInteger64: {
					info.size = 8;
				} break;

				case language::typesystem::TypeDescriptor::typeUInteger64: {
					info.size = 8;
				} break;

				case language::typesystem::TypeDescriptor::typeInteger128: {
					info.size = 16;
				} break;

				case language::typesystem::TypeDescriptor::typeUInteger128: {
					info.size = 16;
				} break;

				case language::typesystem::TypeDescriptor::typeFloat:  {
					info.size = 4;
				} break;

				case language::typesystem::TypeDescriptor::typeDouble: {
					info.size = 8;
				} break;

				default: {
					info.size = 0;
				} break;
			}
			Runtime::rtInstance.objInfoList.push_back(info);

		}
	}
}