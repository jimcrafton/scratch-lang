#pragma once

//scratch_runtime.h


/*
* please define SCRATCH_RUNTIME_DEBUG to put code into 
* "debug" mode. The presence or lack thereof of 
* DEBUG or _DEBUG isn't pipcked up or guaranteed to
* do this
*/


//#undef SCRATCH_RUNTIME_DEBUG

#ifdef SCRATCH_RUNTIME_DEBUG
#pragma message("Scratch Runtime Lib in DEBUG mode")
#else
#pragma message("Scratch Runtime Lib in RELEASE mode")
#endif //SCRATCH_RUNTIME_DEBUG


#ifdef __cplusplus
extern "C" {
#endif

	extern "C" void* __cdecl memset(void*, int, size_t);
	#pragma intrinsic(memset)

	extern "C" void* __cdecl memcpy(void* dest, const void* src, size_t n);
	#pragma intrinsic(memcpy)

	//types, match to compiletime, but can't include types.h because need to keep things 
	//clean, might be an automated way to do this?

	enum Runtime_TypeDescriptor {
		typeUnknown = 0,
		typeRecord = 1,
		typeClass,
		typeBit1,
		typeInteger8 = 8,
		typeUInteger8 = 9,
		typeInteger16 = 16,
		typeUInteger16 = 17,
		typeShort = typeInteger16,
		typeUShort = typeUInteger16,
		typeInteger32 = 32,
		typeUInteger32 = 33,
		typeInteger64 = 64,
		typeUInteger64 = 65,
		typeInteger128 = 128,
		typeUInteger128 = 129,

		typeDouble32 = 34,
		typeFloat = typeDouble32,
		typeDouble64 = 66,
		typeDouble = typeDouble64,
		typeBool = 100,
		typeString,
		typeArray,
		typeDictionary,
		typeMessage,
		typeNilPtr=0xDEAD,
		typeUnmanagedPtr = 0xBEEF,
	};


	inline bool isTypePrimitive(Runtime_TypeDescriptor td) {
		switch (td) {

		case typeBit1: case typeBool:
		case typeInteger8: case typeUInteger8:
		case typeInteger16: case typeUInteger16:
		case typeInteger32: case typeUInteger32:
		case typeInteger64: case typeUInteger64:
		case typeInteger128: case typeUInteger128:
		case typeDouble32: case typeDouble64: {
			return true;
		}break;

		default: {

		}break;
		}

		return false;
	}

	inline bool isTypeRealNumber(Runtime_TypeDescriptor td) {
		switch (td) {

		case typeDouble32: case typeDouble64: {
			return true;
		}break;

		default: {

		}break;
		}

		return false;
	}

	inline bool isTypeInt(Runtime_TypeDescriptor td) {
		switch (td) {

		case typeInteger8: case typeUInteger8:
		case typeInteger16: case typeUInteger16:
		case typeInteger32: case typeUInteger32:
		case typeInteger64: case typeUInteger64:
		case typeInteger128: case typeUInteger128: {
			return true;
		}break;

		default: {

		}break;
		}

		return false;
	}

	inline bool isTypeUnsigned(Runtime_TypeDescriptor td) {
		switch (td) {
		case typeUInteger8:
		case typeUInteger16:
		case typeUInteger32:
		case typeUInteger64:
		case typeUInteger128: {
			return true;
		}break;

		default: {

		}break;
		}

		return false;
	}




	typedef  bool boolT;


	typedef  char int8T;
	typedef  unsigned char uint8T;
	typedef  uint8T byteT;
	typedef  short int16T;
	typedef  unsigned short uint16T;
	typedef  int int32T;
	typedef  unsigned int uint32T;
	typedef  __int64 int64T;
	typedef  unsigned __int64 uint64T;

	struct Type128 {
		int64T lo;
		int64T hi;
	};

	struct TypeU128 {
		uint64T lo;
		uint64T hi;
	};


	typedef  Type128 int128T;
	typedef  TypeU128 uint128T;


	typedef  float float32T;
	typedef double double64T;



	

	


	//----------------------------------------------------------------------------
	//utils
	// 
	void Runtime_error(int32T errCode);

	void Runtime_printf(const char* fmtStr, ...);

	void Runtime_debug_printf(const char* fmtStr, ...);


	//----------------------------------------------------------------------------
	//memory

	constexpr uint32T Runtime_NoIndx = (uint32T)-1;

	enum Runtime_ObjectFlags{
		Runtime_objFlagsHeap = 0x0001, //if not present then on stack
		Runtime_objFlagsConst = 0x0002, 
		Runtime_objFlagsPrimitive = 0x0004, //int, float, etc		
	};

	struct Runtime_objectinfo_ptr {	

		uint32T infoIdx;
		uint32T flags; //some set of Runtime_ObjectFlags
	};

	typedef void* Runtime_memory_info_handle;


	
	struct Runtime_object {
		void* objectVal;
		Runtime_objectinfo_ptr infoptr;
	};


	bool Runtime_objectinfo_isNil(Runtime_object* self);		

	Runtime_memory_info_handle Runtime_object_info(Runtime_object* self);


	void Runtime_memory_info_init(Runtime_memory_info_handle info);
	Runtime_memory_info_handle Runtime_RuntimeMemory_get_from_heap_ptr(void* mem);

	uint64T Runtime_RuntimeMemory_get_size(void* mem);
	Runtime_TypeDescriptor Runtime_RuntimeMemory_get_type(void* mem);

	void Runtime_setMemType(Runtime_objectinfo_ptr* objInfo, bool onHeap);
	bool Runtime_isMemOnHeap(Runtime_objectinfo_ptr* objInfo);
	bool Runtime_isMemOnStack(Runtime_objectinfo_ptr* objInfo);


	void* Runtime_alloc(uint64T size, Runtime_TypeDescriptor type);
	void Runtime_free(void* mem);




	//----------------------------------------------------------------------------
	//arrays

	//meta data about array data
	struct Runtime_array_info {
		//type descriptor for individual elements
		Runtime_TypeDescriptor elementType;

		//length in bytes for each element
		uint64T stride;

	};



	
	typedef void* Runtime_array_handle;



	bool Runtime_array_init(Runtime_array_handle arr);

	Runtime_array_handle Runtime_array_new_empty(Runtime_TypeDescriptor type);
	Runtime_array_handle Runtime_array_new( uint64T size, Runtime_TypeDescriptor type);	
	Runtime_array_handle Runtime_array_new_copy(Runtime_array_handle rhs);
	Runtime_array_handle Runtime_array_new_from_stack(void* data, uint64T size, Runtime_TypeDescriptor type);

	void Runtime_array_delete(Runtime_array_handle self);

	//[] access
	void* Runtime_array_data(Runtime_array_handle self);

	void* Runtime_array_at(Runtime_array_handle self, uint64T index);
	void* Runtime_array_begin(Runtime_array_handle self);
	void* Runtime_array_end(Runtime_array_handle self);
	
	bool Runtime_array_empty(Runtime_array_handle self);

	uint64T Runtime_array_size(Runtime_array_handle self);
	uint64T Runtime_array_capacity(Runtime_array_handle self);
	uint64T Runtime_array_size_bytes(Runtime_array_handle self);
	Runtime_TypeDescriptor Runtime_array_type(Runtime_array_handle self);

	void Runtime_array_clear(Runtime_array_handle self);

	void Runtime_array_reserve(Runtime_array_handle self, uint64T newCapacity);

	Runtime_array_handle Runtime_array_resize(Runtime_array_handle self, uint64T newSize);

	void Runtime_array_append(Runtime_array_handle self, void* newElement);
	void Runtime_array_append_from(Runtime_array_handle self, Runtime_array_handle src);

	void Runtime_array_insert(Runtime_array_handle self, void* newElement, uint64T insertAt);

	void Runtime_array_copy(Runtime_array_handle self, Runtime_array_handle src);

	void Runtime_array_bytes_copy(Runtime_array_handle self, Runtime_array_handle src);


	enum Runtime_array_cmp_result{
		rtArrayCmpEqual = 0,
		rtArrayCmpGt = 1,
		rtArrayCmpLt = -1,
		rtArrayCmpInvalid = -9999,
	};

	Runtime_array_cmp_result Runtime_array_compare(Runtime_array_handle self, Runtime_array_handle rhs);

	//----------------------------------------------------------------------------



	//----------------------------------------------------------------------------
	//hashtable
	//any key, any value
	typedef void* Runtime_hashtable_handle;

	



	Runtime_hashtable_handle Runtime_hashtable_new(Runtime_TypeDescriptor keyType, Runtime_TypeDescriptor valType);
	void Runtime_hashtable_delete(Runtime_hashtable_handle self);


	bool Runtime_hashtable_empty(Runtime_hashtable_handle self);
	uint64T Runtime_hashtable_size(Runtime_hashtable_handle self);
	
	uint64T Runtime_hashtable_capacity(Runtime_hashtable_handle self);
	void Runtime_hashtable_set_capacity(Runtime_hashtable_handle self, uint64T capacity);

	double Runtime_hashtable_max_usage_factor(Runtime_hashtable_handle self);
	void Runtime_hashtable_set_max_usage_factor(Runtime_hashtable_handle self, double val);

	double Runtime_hashtable_capacity_grow_by(Runtime_hashtable_handle self);
	void Runtime_hashtable_set_capacity_grow_by(Runtime_hashtable_handle self, double val);


	void Runtime_hashtable_clear(Runtime_hashtable_handle self);	

	void Runtime_hashtable_insert(Runtime_hashtable_handle self, void* key, void* val);
	void Runtime_hashtable_erase(Runtime_hashtable_handle self, void* key);

	

	//[] access
	void* Runtime_hashtable_at(Runtime_hashtable_handle self, void* key);
	
	//----------------------------------------------------------------------------



	//----------------------------------------------------------------------------
	//strings
	typedef int8T charT;
	

	

	typedef void* Runtime_string_handle;

	Runtime_string_handle Runtime_string_new_empty();
	Runtime_string_handle Runtime_string_new(const charT* c_strPtr);
	Runtime_string_handle Runtime_string_new_with_size(const charT* c_strPtr, uint64T size);
	Runtime_string_handle Runtime_string_new_char_count(charT ch, uint64T count);
	Runtime_string_handle Runtime_string_new_copy(Runtime_string_handle rhs);

	void Runtime_string_clear(Runtime_string_handle self);

	void Runtime_string_delete(Runtime_string_handle self);

	bool Runtime_string_empty(Runtime_string_handle self);
	uint64T Runtime_string_size(Runtime_string_handle self);
	uint64T Runtime_string_size_bytes(Runtime_string_handle self);

	uint64T Runtime_string_hash(Runtime_string_handle self);
	void Runtime_string_assign(Runtime_string_handle self, charT* c_strPtr);
	void Runtime_string_assign_with_size(Runtime_string_handle self, charT* c_strPtr, uint64T size);
	void Runtime_string_assign_char_count(Runtime_string_handle self, charT ch, uint64T count);
	void Runtime_string_assign_copy(Runtime_string_handle self, Runtime_string_handle rhs);
	charT Runtime_string_at(Runtime_string_handle self, uint64T index);
	void Runtime_string_set(Runtime_string_handle self, uint64T index, charT ch);

	int32T Runtime_string_compare(Runtime_string_handle self, Runtime_string_handle rhs);

	Runtime_string_handle Runtime_string_substr(Runtime_string_handle self, uint64T start, uint64T count);

	uint64T Runtime_string_find(Runtime_string_handle self, Runtime_string_handle searchStr);

	charT* Runtime_string_get_cstr(Runtime_string_handle self);

	//----------------------------------------------------------------------------


	//----------------------------------------------------------------------------
	//dictionary
	// specialization of hashtable
	//key == string, any value type
	typedef void* Runtime_dictionary_handle;	

	Runtime_dictionary_handle Runtime_dictionary_new(Runtime_TypeDescriptor valType);
	void Runtime_dictionary_delete(Runtime_dictionary_handle self);


	bool Runtime_dictionary_empty(Runtime_dictionary_handle self);
	uint64T Runtime_dictionary_size(Runtime_dictionary_handle self);
	uint64T Runtime_dictionary_capacity(Runtime_dictionary_handle self);
	void Runtime_dictionary_set_capacity(Runtime_dictionary_handle self, uint64T capacity);
	void Runtime_dictionary_clear(Runtime_dictionary_handle self);

	void Runtime_dictionary_insert(Runtime_dictionary_handle self, Runtime_string_handle key, void* val);
	void Runtime_dictionary_erase(Runtime_dictionary_handle self, Runtime_string_handle key);



	//[] access
	void* Runtime_dictionary_at(Runtime_dictionary_handle self, Runtime_string_handle key);
	


	//----------------------------------------------------------------------------
	//runtime environment methods
	//startup
	int32T Runtime_init();

	int32T Runtime_main_entry_point();

	//termination
	void Runtime_terminate();

	//

#ifdef __cplusplus
} // end extern "C"
#endif


