#include "datatypes.h"
#include <unordered_map>
#include "runtime.h"

namespace datatypes {

	class string_storage {
	public:
		std::string data;
		uint64_t refcount = 0;
		uint64_t size = 0;
		runtime::StringPool* pool = nullptr;
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

		static uint64_t hash_impl(const datatypes::string::charT* c_strPtr, size_t len) {
			return std::hash<std::string>{}(std::string(c_strPtr, len));
		}
	};
}

namespace runtime {
	datatypes::string_storage* StringPool::allocate()
	{
		datatypes::string_storage* result = nullptr;

		result = new datatypes::string_storage();

		return result;
	}

	datatypes::string_storage* StringPool::allocate(const datatypes::string::charT* c_strPtr, size_t len) 
	{
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

	void StringPool::deallocate(datatypes::string_storage* str) 
	{
		printf("deallocating string instance %p... \n", str);
	}
}

	




namespace datatypes {


	string::string()
	{
		data = runtime::StringPool::allocate();
	}

	string::~string()
	{
		string_storage* ptr = (string_storage*)data;

		runtime::StringPool::deallocate(ptr);
	}

	string::string(const string& rhs) :
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
		data = runtime::StringPool::allocate(v.c_str(), v.length());
	}

	void string::assign(const charT* c_strPtr)
	{
		string_storage* ptr = (string_storage*)data;
		if (nullptr != ptr) {
			delete ptr;
		}

		data = runtime::StringPool::allocate(c_strPtr, strlen(c_strPtr));
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
		return runtime::Runtime::getInfo(*this);
	}


}