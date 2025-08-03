#include "scratch_runtime.h"

//#include <cstdio>
#include <Windows.h>
#include <functional>
#include <string>


/*
* if we want to support classes here, then we need something like this
* lots of work to support
void* operator new (uint64T size) 
{
    void* result = nullptr;

    result = HeapAlloc(GetProcessHeap(), 0, size);

    return result;
}

class foo {
    int a = 0;
};
*/



#ifdef __cplusplus
extern "C" {
#endif
	int _fltused = 0;  //why does this matter? For double??
}

#if defined(_WIN64)	
constexpr size_t Runtime_FNV_prime = 1099511628211ULL;
#else 	
constexpr size_t Runtime_FNV_prime = 16777619U;
#endif


//platform/os calls
void Win32_printf_arglist(const char* fmtStr, va_list argList);
void Win32_debug_printf_arglist(const char* fmtStr, va_list argList);
void Win32_debug_printf(const char* fmtStr,...);
void Win32_Runtime_error(int32T err);
void Win32_Memory_init(void* mem, uint64T size);
void* Win32_Runtime_alloc(uint64T size);
void Win32_Runtime_free(void* mem);
char** Win32_get_command_line(int32T* argcPtr);
void Win32_Runtime_assert(const char* msg, const char* functionName, const char* filename, uint64T lineno );





#ifdef SCRATCH_RUNTIME_DEBUG	

	#define RUNTIME_ASSERT(condition)  ((void)(                                                       \
            (!!(condition)) ||                                                               \
            (Win32_Runtime_assert(#condition, __FUNCTION__ , __FILE__, (unsigned)(__LINE__)), 0)) \
        )
#else
	#define RUNTIME_ASSERT(condition) ((void)0)
#endif //SCRATCH_RUNTIME_DEBUG



//CRT memcpy/memset replacements because we've 
//turned off the CRT, and if we turn 02 optimization
//then memset/memcpy __HAVE__ to be defined somewhere
//shjould figure out a much better implementation than
//this copy/pasted slop from online
#pragma function(memset)
void* __cdecl memset(void* pTarget, int value, size_t cbTarget) {
	unsigned char* p = static_cast<unsigned char*>(pTarget);
	while (cbTarget-- > 0) {
		*p++ = static_cast<unsigned char>(value);
	}
	return pTarget;
}


#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t n)
{
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;

	// Copy byte by byte
	while (n--) {
		*d++ = *s++;
	}

	return dest;
}


struct Runtime_memory_info {
	Runtime_TypeDescriptor type;

	//size in bytes for the object. for stack based 
	//primitive this would be the storage required for
	//the object, i.e. int32T would be 4, 
	//uint8T wiould be 1, etc
	uint64T size;
	void* memPtr;
};



struct Runtime_heap_array {

	//number of elements in array
	//total byte size is size * stride
	uint64T size;
	uint64T capacity;

	//data that holds contiguous elements in memory
	//where each element is stride bytes in length
	void* data;

	//info about the array, potentially
	//shared amongst array instances
	Runtime_array_info* infoPtr;
};


struct Runtime_stack_array {

	//number of elements in array
	//total byte size is size * stride
	uint64T size;
	//data that holds contiguous elements in memory
	//where each element is stride bytes in length
	void* data;
	//info about the array, potentially
	//shared amongst array instances
	Runtime_array_info* infoPtr;
};


#define RUNTIME_HEAP_ARRAY(arr) ((Runtime_heap_array*)((Runtime_array*)arr)->internalData)
#define RUNTIME_STACK_ARRAY(arr) ((Runtime_stack_array*)((Runtime_array*)arr)->internalData)

//meta data about array data
struct Runtime_hashtable_info {
	//key/value descriptors 
	Runtime_TypeDescriptor keyType;
	Runtime_TypeDescriptor valueType;

	//length in bytes for each key/value
	uint64T keyStride;
	uint64T valStride;
};




struct Runtime_Instance {
	Runtime_array_info* arrayInfoList;
	uint32T arrayInfoListSize;
	uint32T initialArrayInfoSize;


	Runtime_hashtable_info* hashtableInfoList;
	uint32T hashtableInfoListSize;
	uint32T initialHashtableInfoSize;
};

Runtime_Instance* runtimeInstancePtr = nullptr;
Runtime_memory_info nilInfo;
uint64T totalBytesHeapAllocated;

#define RUNTIME_ARRAY_CAPACITY_MULTIPLIER	1.5
#define RUNTIME_ARRAY_CAPACITY_SMALL_MULTIPLIER	2.0


#define WIN32_MAX_PRINTF_BUFFER_SIZE  1024 


//----------------------------------------------------------------------------
//platform specific stuff




void Win32_printf_arglist(const char* fmtStr, va_list argList)
{
	char buf[WIN32_MAX_PRINTF_BUFFER_SIZE];
	int res = wvsprintfA(buf, fmtStr, argList);
	if (res > 0 && res < (sizeof(buf) - 1)) {

		buf[res] = 0;
		DWORD done;
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, lstrlen(buf), &done, NULL);
	}

}


void Win32_debug_printf_arglist(const char* fmtStr, va_list argList)
{
	char buf[WIN32_MAX_PRINTF_BUFFER_SIZE];
	int res = wvsprintfA(buf, fmtStr, argList);
	if (res > 0 && res < (sizeof(buf) - 1)) {
		buf[res] = 0;	

		DWORD done;
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, lstrlen(buf), &done, NULL);

		OutputDebugStringA(buf);
	}
}

void Win32_debug_printf(const char* fmtStr, ...)
{
	va_list argList;
	va_start(argList, fmtStr);

	Win32_debug_printf_arglist(fmtStr, argList);

	va_end(argList);
}


void Win32_Runtime_error(int32T err)
{
	Runtime_printf("Exiting with error code : %d\n", err);
	ExitProcess(err);
}


void Win32_Memory_init(void* mem, uint64T size)
{
	RtlSecureZeroMemory(mem, size);
}



void* Win32_Runtime_alloc(uint64T size)
{
	void* result = nullptr;

	result = HeapAlloc(GetProcessHeap(), 0, size);



	return result;
}

void Win32_Runtime_free(void* mem)
{
	if (!HeapFree(GetProcessHeap(), 0, mem)) {
		Runtime_debug_printf("Unable to free : %p\n", mem);
		Runtime_error(-1);
	}
}



char** Win32_get_command_line(int32T* argcPtr)
{

	LPWSTR cmdLinePtr = GetCommandLineW();
	int argc = 0;
	int8T** argv = nullptr;

	if (nullptr == cmdLinePtr) {
		Runtime_printf("unable to get command line from Windows\n");
		Runtime_error(-1);
	}
	else {
		LPWSTR* argvTmp = CommandLineToArgvW(cmdLinePtr, &argc);
		bool failedToTranslate = false;
		int charsToAllocate = 0;
		for (int i = 0; i < argc; i++) {
			auto res = WideCharToMultiByte(CP_UTF8, 0, argvTmp[i], -1, NULL, 0, NULL, NULL);
			if (res == 0) {
				//failure. exit
				auto errCode = GetLastError();
				Runtime_debug_printf("WideCharToMultiByte failed, err: %d \n", errCode);
				failedToTranslate = true;
				break;

			}
			charsToAllocate += res + 1;
		}

		argv = (int8T**)Win32_Runtime_alloc((argc + 1) * sizeof(char*) + charsToAllocate);

		if (nullptr == argv) {
			auto errCode = GetLastError();
			Runtime_debug_printf("alloc failed for ansi transform buffer err: %d \n", errCode);
			failedToTranslate = true;
		}
		else {

			int8T* arg = (int8T*)&((argv)[argc + 1]);
			for (int i = 0; i < argc; i++)
			{
				argv[i] = arg;
				auto res = WideCharToMultiByte(CP_UTF8, 0, argvTmp[i], -1, arg, charsToAllocate, NULL, NULL);
				if (res == 0) {
					//failure. exit
					auto errCode = GetLastError();
					Runtime_debug_printf("WideCharToMultiByte failed, err: %d \n", errCode);
					failedToTranslate = true;
					break;

				}

				arg += res + 1;
			}
			argv[argc] = nullptr;
		}


		LocalFree(argvTmp);

		if (failedToTranslate) {
			Runtime_error(-1);
		}

		*argcPtr = argc;
	}

	return argv;
}


void Win32_Runtime_assert(const char* msg, const char* functionName, const char* filename, uint64T lineno)
{
	Runtime_printf("ASSERT: [%s] (%s) %s:%I64u\n", msg, functionName, filename, lineno);
	Runtime_error(-10);
}


//end of platform specific stuff
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
//utils


uint64T Runtime_calc_stride_for_type(Runtime_TypeDescriptor type)
{
	uint64T result = 0;

	switch (type) {
		case typeRecord: case typeClass: {
			result = sizeof(void*);
		}break;

		case typeInteger8: case typeUInteger8: {
			result = sizeof(int8T);
		}break;

		case typeInteger16: case typeUInteger16: {
			result = sizeof(int16T);
		}break;

		case typeInteger32: case typeUInteger32: {
			result = sizeof(int32T);
		}break;

		case typeInteger64: case typeUInteger64: {
			result = sizeof(int64T);
		}break;

		case typeInteger128: case typeUInteger128: {
			result = sizeof(int128T);
		}break;

		case typeDouble32: {
			result = sizeof(float32T);
		}break;

		case typeDouble64: {
			result = sizeof(double64T);
		}break;

		case typeBool: {
			result = sizeof(bool);
		}break;

		case typeString: {
			result = sizeof(void*);
		}break;

		case typeArray: {
			result = sizeof(void*);
		}break;

		case typeDictionary: {
			result = sizeof(void*);
		}break;

		case typeMessage: {
			result = sizeof(void*);
		}break;

		case typeUnknown: {
			result = sizeof(void*);
		}break;

		default: {
			result = 0;
		}break;
	}

	return result;
}



void Runtime_printf_arglist(const char* fmtStr, va_list argList)
{
	Win32_printf_arglist(fmtStr, argList);
}



void Runtime_debug_printf(const char* fmtStr, ...)
{
#ifdef SCRATCH_RUNTIME_DEBUG
	Win32_debug_printf("DEBUG ");
	va_list argList;
	va_start(argList, fmtStr);

	Win32_debug_printf_arglist(fmtStr, argList);

	va_end(argList);
#endif
}


void Runtime_printf(const char* fmtStr, ...)
{

	va_list argList;
	va_start(argList, fmtStr);

	Runtime_printf_arglist(fmtStr, argList);

	va_end(argList);
}




void Runtime_error(int32T errCode)
{
	Win32_Runtime_error(errCode);
}


//utils end
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
//memory


extern "C" void my_mem_copy(void* dest, const void* src, size_t count);






uint64T Runtime_mem_cpy(void* src, void* dest, uint64T size)
{
	if (0 == size) {
		return 0;
	}
	//TODO this should be in optimized assembly....
	uint64T result = 0;
	if (src != dest) {
		uint8T* srcStart = (uint8T*) src;
		uint8T* srcEnd = srcStart + size;
		uint8T* destStart = (uint8T*)dest;
		while (srcStart< srcEnd) {
			*destStart = *srcStart;
			srcStart++;
			destStart++;
			result++;
		}
	}
	return result;
}

int Runtime_mem_cmp(void* lhs, uint64T lhsSize, void* rhs, uint64T rhsSize)
{
	if (lhs == rhs) {
		return 0;
	}

	if (nullptr == lhs && nullptr != rhs) {
		return 1;
	}
	else if (nullptr != lhs && nullptr == rhs) {
		return -1;
	}
	else if (lhsSize < rhsSize) {
		return -1;
	}
	else if (lhsSize > rhsSize) {
		return 1;
	}

	auto lhsPtr = (uint8T*)lhs;
	auto lhsEnd = lhsPtr + lhsSize;
	auto rhsPtr = (uint8T*)rhs;
	while (lhsPtr < lhsEnd) {
		if (*lhsPtr < *rhsPtr) {
			return -1;
		}
		else if (*lhsPtr > *rhsPtr) {
			return 1;
		}
		lhsPtr++;
		rhsPtr++;
	}

	return 0;
}



uint64T Runtime_hash_basic_bytes(uint8T* data, uint64T size)
{
	uint64T result = 0;

	auto ptr = data;
	for (uint64T i = 0; i < size; ++i) {
		result ^= (uint64T)*ptr;
		result *= Runtime_FNV_prime;
		ptr++;
	}

	return result;
}



void Runtime_Memory_init(void* mem, uint64T size)
{
	Win32_Memory_init(mem, size);
}

void Runtime_memory_info_init(Runtime_memory_info_handle info)
{
	Runtime_Memory_init(info, sizeof(Runtime_memory_info));
}

class objectinfo_flags {
public:
	enum FlagVals {
		flagMemOnStack = 0,
		flagMemOnHeap = 1
	};


	objectinfo_flags() : memtype(flagMemOnStack), unused1(0), unused2(0), unused3(0) {}
	objectinfo_flags(const uint32T& rhs) : memtype(0), unused1(0), unused2(0), unused3(0) {
		*this = rhs;
	}

	inline void setMemType(bool onHeap) { memtype = onHeap ? flagMemOnHeap : flagMemOnStack; }
	inline bool isMemOnHeap() const { return memtype == flagMemOnHeap ? true : false; }
	inline bool isMemOnStack() const { return memtype == flagMemOnStack ? true : false; }

	inline operator uint32T () const { return  (uint32T)(*((uint32T*)this)); }

	inline objectinfo_flags& operator=(const uint32T& rhs) {   
		(uint32T&)(*((uint32T*)this)) = rhs;
		return* this;
	}
	uint8T memtype : 1; //flagMemOnStack or flagMemOnHeap
	uint8T unused1 : 7;
	uint8T unused2 : 8;
	uint16T unused3 : 16;
};





Runtime_memory_info* Runtime_objectinfo_nil()
{
	return &nilInfo;
}

bool Runtime_objectinfo_isNil(Runtime_object* self)
{
	auto n = *Runtime_objectinfo_nil();

	auto memInfo = (Runtime_memory_info*)Runtime_object_info(self);

	return memInfo->size == n.size && memInfo->type == n.type;
}



Runtime_memory_info_handle Runtime_object_info(Runtime_object* self)
{
	Runtime_memory_info* result = nullptr;

	return result;
}


void Runtime_setMemType(Runtime_objectinfo_ptr* objInfo, bool onHeap)
{
	objectinfo_flags flags = objInfo->flags;
	flags.setMemType(onHeap);
	objInfo->flags = flags;
}

bool Runtime_isMemOnHeap(Runtime_objectinfo_ptr* objInfo)
{
	objectinfo_flags flags = objInfo->flags;

	return flags.isMemOnHeap();
}

bool Runtime_isMemOnStack(Runtime_objectinfo_ptr* objInfo)
{
	return true;
}

//memory end
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
//array 



enum Runtime_Array_Flags {
	rtArrayDynamic = 0x0000,
	rtArrayStatic = 0x0001
};


struct Runtime_array {
	uint32T flags;
	void* internalData;
};





Runtime_array_info* Runtime_get_array_info_for_type(Runtime_TypeDescriptor type)
{
	Runtime_array_info* result = nullptr;
	for (uint32T i = 0; i < runtimeInstancePtr->arrayInfoListSize; i++) {
		if (type == runtimeInstancePtr->arrayInfoList[i].elementType) {
			result = &runtimeInstancePtr->arrayInfoList[i];
			break;
		}
	}

	return result;
}



bool Runtime_heap_array_init(Runtime_heap_array* arr)
{
	
	Runtime_Memory_init(arr, sizeof(Runtime_heap_array));

	return true;
}

bool Runtime_stack_array_init(Runtime_stack_array* arr)
{

	Runtime_Memory_init(arr, sizeof(Runtime_stack_array));

	return true;
}


bool Runtime_array_init(Runtime_array_handle arr)
{
	Runtime_array* arrPtr = (Runtime_array*)arr;
	return (arrPtr == nullptr) ? false :
		(arrPtr->flags == rtArrayDynamic) ?
		Runtime_heap_array_init(RUNTIME_HEAP_ARRAY(arr)) :
		Runtime_stack_array_init( RUNTIME_STACK_ARRAY(arr) );
}

void Runtime_array_heap_free_data(Runtime_array_handle self)
{

	if (nullptr == self) {
		return;
	}

	Runtime_array* selfPtr = (Runtime_array*)self;

	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);

	if (nullptr == internArr->data || 0 == internArr->size) {
		return;
	}

	switch (internArr->infoPtr->elementType) {
		case typeClass: {

		} break;

		case typeRecord: {

		} break;

		case typeArray: {
			auto sz = Runtime_array_size(self);
			for (uint64T i = 0; i < sz; i++) {
				Runtime_array_handle arr = (Runtime_array_handle)Runtime_array_at(self, i);
				Runtime_array_delete(arr);
			}
		} break;

		case typeDictionary: {

		} break;

		default: {
			Runtime_free(internArr->data);
		} break;
	}
	internArr->data = nullptr;
}

void Runtime_array_heap_reserve(Runtime_array_handle self)
{
	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);

	if (0 == internArr->capacity) {
		
		Runtime_array_heap_free_data(self);

		return;
	}

	auto newData = Runtime_alloc(internArr->infoPtr->stride * internArr->capacity, internArr->infoPtr->elementType);
	Runtime_Memory_init(newData, internArr->infoPtr->stride * internArr->capacity);

	if (nullptr != internArr->data) {
		uint8T* elem = (uint8T*)internArr->data;
		uint8T* destElem = (uint8T*)newData;

		Runtime_mem_cpy(elem, destElem, internArr->infoPtr->stride * internArr->size);

		Runtime_array_heap_free_data(self);
	}

	internArr->data = newData;
}


Runtime_array_handle Runtime_array_new_struct(uint32T flags)
{
	Runtime_array* result = nullptr;
	uint64T allocSize = sizeof(Runtime_array);

	if (flags == rtArrayDynamic) {
		allocSize += sizeof(Runtime_heap_array);
	}
	else if (flags == rtArrayStatic) {
		allocSize += sizeof(Runtime_stack_array);
	}

	auto memPtr = Runtime_alloc(allocSize, typeArray);
	result = (Runtime_array*)memPtr;
	result->flags = flags;

	if (flags == rtArrayDynamic) {		
		Runtime_heap_array* internArr = (Runtime_heap_array*)(((uint8T*)result) + sizeof(Runtime_array));
		result->internalData = internArr;
	}
	else if (flags == rtArrayStatic) {
		Runtime_stack_array* internArr = (Runtime_stack_array*)(((uint8T*)result) + sizeof(Runtime_array));
		result->internalData = internArr;
	}

	return (Runtime_array_handle) result;
}

Runtime_array_handle Runtime_array_new_empty(Runtime_TypeDescriptor type)
{
	Runtime_array_handle result = Runtime_array_new_struct(rtArrayDynamic);

	if (!Runtime_array_init(result)) {
		return nullptr;
	}
	
	Runtime_array* arr = (Runtime_array*)result;
	if (arr->flags == rtArrayDynamic) {
		Runtime_heap_array* internArr = (Runtime_heap_array * )arr->internalData;
		internArr->infoPtr = Runtime_get_array_info_for_type(type);

		internArr->size = 0;
		internArr->capacity = 0;
		internArr->data = nullptr;
	}
	else if (arr->flags == rtArrayStatic) {
		Runtime_stack_array* internArr = (Runtime_stack_array*)arr->internalData;
		internArr->infoPtr = Runtime_get_array_info_for_type(type);

	}

	return result;
}

Runtime_array_handle Runtime_array_new(uint64T size, Runtime_TypeDescriptor type)
{
	Runtime_array_handle result = Runtime_array_new_empty(type);
	Runtime_heap_array* internArr = (Runtime_heap_array*)(((uint8T*)result) + sizeof(Runtime_array));	
	internArr->size = size;	
	internArr->capacity = ((double)size * (size < 4 ? RUNTIME_ARRAY_CAPACITY_SMALL_MULTIPLIER : RUNTIME_ARRAY_CAPACITY_MULTIPLIER));

	Runtime_array_heap_reserve(result);

	return result;
}

Runtime_array_handle Runtime_array_new_copy(Runtime_array_handle rhs)
{
	Runtime_heap_array* rhsInternArr = RUNTIME_HEAP_ARRAY(rhs);

	Runtime_array_handle result = Runtime_array_new(rhsInternArr->size, rhsInternArr->infoPtr->elementType);

	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(result);

	Runtime_mem_cpy(rhsInternArr->data, internArr->data, rhsInternArr->size * rhsInternArr->infoPtr->stride);

	return result;
}

Runtime_array_handle Runtime_array_new_from_stack(void* data, uint64T size, Runtime_TypeDescriptor type)
{
	Runtime_array* result = (Runtime_array*) Runtime_array_new_struct(rtArrayStatic);

	auto stackData = RUNTIME_STACK_ARRAY(result);

	stackData->size = size;
	stackData->infoPtr = Runtime_get_array_info_for_type(type);
	stackData->data = data;

	return result;
}

void Runtime_array_clear(Runtime_array_handle self)
{	
	Runtime_array* selfArr = (Runtime_array * )self;

	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);

	if (rtArrayDynamic == selfArr->flags) {
		Runtime_array_heap_free_data(self);
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		internArr->size = 0;
	}
	else if (rtArrayStatic == selfArr->flags) {

	}
}

void Runtime_array_delete(Runtime_array_handle self)
{		
	//Runtime_debug_printf("Runtime_array_delete %p size: %d cap: %d\n", self, (int)Runtime_array_size(self), (int)Runtime_array_capacity(self));
	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayDynamic == selfArr->flags) {
		Runtime_array_clear(self);
	}
	else {
		
	}
	
	Runtime_free(self);
}

void* Runtime_array_begin(Runtime_array_handle self)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayDynamic == selfArr->flags) {
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		return internArr->data;
	}
	else if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		return internArr->data;
	}
	return nullptr;
}

void* Runtime_array_end(Runtime_array_handle self)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayDynamic == selfArr->flags) {
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		return (void*)((uint8T*)internArr->data + internArr->size * internArr->infoPtr->stride);
	}
	else if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		return (void*)((uint8T*)internArr->data + internArr->size * internArr->infoPtr->stride);
	}
	return nullptr;

}

void* Runtime_array_at(Runtime_array_handle self, uint64T index)
{
	void* result = nullptr;
	Runtime_array* selfArr = (Runtime_array*)self;

	if (rtArrayDynamic == selfArr->flags) {
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		result = ((uint8T*)internArr->data) + index * internArr->infoPtr->stride;
	}
	else if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		result = ((uint8T*)internArr->data) + index * internArr->infoPtr->stride;
	}
	return result;
}


Runtime_array_info* Runtime_array_get_info(Runtime_array_handle self)
{
	Runtime_array_info* result = nullptr;

	Runtime_array* selfArr = (Runtime_array*)self;

	if (rtArrayDynamic == selfArr->flags) {
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		result = internArr->infoPtr;
	}
	else if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		result = internArr->infoPtr;
	}

	return result;
}

bool Runtime_array_empty(Runtime_array_handle self)
{
	return Runtime_array_size(self) == 0 ? true : false;
}

uint64T Runtime_array_size_bytes(Runtime_array_handle self)
{	
	uint64T result = 0;

	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayDynamic == selfArr->flags) {
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		result = (internArr->capacity * internArr->infoPtr->stride);
	}
	else if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		result = (internArr->size * internArr->infoPtr->stride);
	}

	return result;
}

uint64T Runtime_array_size(Runtime_array_handle self)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayDynamic == selfArr->flags) {
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		return internArr->size;
	}
	else if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		return internArr->size;
	}
	
	return 0;
}

void* Runtime_array_data(Runtime_array_handle self)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayDynamic == selfArr->flags) {
		Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
		return internArr->data;
	}
	else if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		return internArr ->data;
	}

	return nullptr;
}


uint64T Runtime_array_capacity(Runtime_array_handle self)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayStatic == selfArr->flags) {
		return 0;
	}
	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);	
	return internArr->capacity;
}

Runtime_TypeDescriptor Runtime_array_type(Runtime_array_handle self)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	if (rtArrayStatic == selfArr->flags) {
		Runtime_stack_array* internArr = RUNTIME_STACK_ARRAY(self);
		return internArr->infoPtr->elementType;
	}

	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
	return internArr->infoPtr->elementType;
}

void Runtime_array_reserve(Runtime_array_handle self, uint64T newCapacity)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);

	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
	 
	if (newCapacity > internArr->capacity) {
		internArr->capacity = newCapacity;
		Runtime_array_heap_reserve(self);
	}
	else {
		internArr->capacity = newCapacity;
	}
}

Runtime_array_handle Runtime_array_resize(Runtime_array_handle self, uint64T newSize)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);


	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
	Runtime_array_handle result = self;
	
	if (newSize >= internArr->capacity) {
		if ((newSize - internArr->capacity) > internArr->capacity) {
			internArr->capacity = newSize; 
		}

		internArr->capacity = ((double)internArr->capacity) * RUNTIME_ARRAY_CAPACITY_MULTIPLIER;
		Runtime_array_heap_reserve(self);
		internArr->size = newSize;
	}

	return result;
}

void Runtime_array_append(Runtime_array_handle self, void* newElement)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);

	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
	Runtime_array_insert(self, newElement, internArr->size);
}

void Runtime_array_append_from(Runtime_array_handle self, Runtime_array_handle src)
{
	if (nullptr == self || nullptr == src) {
		Runtime_debug_printf("self or src arrays are null");
		return;
	}

	Runtime_array* selfArr = (Runtime_array*)self;
	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);

	
	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);


	auto srcSize = Runtime_array_size(src);
	auto srcCapacity = Runtime_array_capacity(src);
	auto srcInfo = Runtime_array_get_info(src);

	if (srcSize == 0) {
		return;
	}

	if (internArr->infoPtr->elementType != srcInfo->elementType) {
		Runtime_debug_printf("Can't append from different types of arrays");
		return;
	}

	if ((internArr->size + srcSize) == internArr->capacity) {
		internArr->capacity = ((double)(internArr->capacity + srcCapacity)) * RUNTIME_ARRAY_CAPACITY_MULTIPLIER;
		Runtime_array_heap_reserve(self);
	}

	auto destPtr = Runtime_array_at(self, internArr->size);
	auto srcPtr = Runtime_array_at(src, 0);


	Runtime_mem_cpy(srcPtr, destPtr, srcSize * internArr->infoPtr->stride);

	internArr->size += srcSize;
}

void Runtime_array_insert(Runtime_array_handle self, void* newElement, uint64T insertAt)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);


	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);

	if (internArr->size == internArr->capacity) {
		internArr->capacity = ((double)internArr->capacity) * RUNTIME_ARRAY_CAPACITY_MULTIPLIER;
		Runtime_array_heap_reserve(self);
	}
	auto oldSz = internArr->size;
	


	auto ptr = Runtime_array_at(self, insertAt);
	auto src = (uint8T*)ptr;
	

	auto end = src;

	src = (uint8T*)Runtime_array_end(self);

	auto dest = src + internArr->infoPtr->stride; //one forward

	while (src> end) {

		*dest = *src;

		dest--;
		src--;
	}
	

	Runtime_mem_cpy(newElement, ptr, internArr->infoPtr->stride);

	internArr->size++;
}

void Runtime_array_bytes_copy(Runtime_array_handle self, Runtime_array_handle src)
{
	Runtime_array* selfArr = (Runtime_array*)self;
	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);


	auto srcInfo = Runtime_array_get_info(src);
	
	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);

	RUNTIME_ASSERT(srcInfo->elementType == internArr->infoPtr->elementType);

	auto srcData = Runtime_array_data(src);	

	Runtime_mem_cpy(srcData, internArr->data, internArr->size * internArr->infoPtr->stride);
}

void Runtime_array_copy(Runtime_array_handle self, Runtime_array_handle src)
{
	if (nullptr == self || nullptr == src) {
		Runtime_debug_printf("self or src arrays are null");
		return;
	}

	Runtime_array* selfArr = (Runtime_array*)self;
	RUNTIME_ASSERT(rtArrayDynamic == selfArr->flags);


	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);


	auto srcSize = Runtime_array_size(src);
	auto srcCapacity = Runtime_array_capacity(src);
	auto srcInfo = Runtime_array_get_info(src);



	if (srcSize == 0) {
		return;
	}

	Runtime_array_clear(self);

	internArr->infoPtr = srcInfo;

	if ((srcSize) == internArr->capacity) {
		
		internArr->capacity = ((double)(internArr->capacity + srcCapacity)) * RUNTIME_ARRAY_CAPACITY_MULTIPLIER;
		Runtime_array_heap_reserve(self);
	}
	internArr->size = srcSize;

	
	Runtime_array_bytes_copy(self,src);
}

Runtime_array_cmp_result Runtime_array_compare(Runtime_array_handle self, Runtime_array_handle rhs)
{
	Runtime_array_cmp_result result = rtArrayCmpEqual;

	Runtime_heap_array* internArr = RUNTIME_HEAP_ARRAY(self);
	Runtime_heap_array* rhsInternArr = RUNTIME_HEAP_ARRAY(rhs);

	if (internArr->infoPtr->elementType != rhsInternArr->infoPtr->elementType) {
		return rtArrayCmpInvalid;
	}

	switch (internArr->infoPtr->elementType) {
		case typeClass: {

		} break;

		case typeRecord: {

		} break;

		case typeArray: {
			auto sz = Runtime_array_size(self);
			//for (uint64T i = 0; i < sz; i++) {
				//Runtime_array_handle arr = (Runtime_array_handle)Runtime_array_at(self, i);
				//Runtime_array_delete(arr);
			//}

		} break;

		case typeDictionary: {

		} break;

		default: {
			auto sz = internArr->size * internArr->infoPtr->stride;
			auto szRhs = rhsInternArr->size * rhsInternArr->infoPtr->stride;
			result = (Runtime_array_cmp_result)Runtime_mem_cmp(internArr->data, sz, rhsInternArr->data, szRhs);

		} break;
	}

	

	return result;
}

//array end
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//strings

struct Runtime_string {

	void* internalData;
};

uint64T Runtime_c_str_length(const charT* c_strPtr)
{
	uint64T result = 0;
	const charT* ptr = c_strPtr;
	while (*ptr != 0) {
		ptr++;
		result++;
	}
	return result;
}


struct Runtime_string_internal_data {
	Runtime_array_handle strData;
	uint64T hashval;
};

#define RUNTIME_STRING_INTERNAL_DATA(self) (Runtime_string_internal_data*)((Runtime_string*)self)

Runtime_string_handle Runtime_string_new_empty()
{
	Runtime_string_handle result = nullptr;
	auto memPtr = Runtime_alloc(sizeof(Runtime_string) + sizeof(Runtime_string_internal_data), typeString);
	result = (Runtime_string_handle)memPtr;
	Runtime_Memory_init(result, sizeof(Runtime_string) + sizeof(Runtime_string_internal_data));

	((Runtime_string*)result)->internalData = ((uint8T*)memPtr) + sizeof(Runtime_string);

	return result;
}

Runtime_string_handle Runtime_string_new(const charT* c_strPtr)
{
	
	return Runtime_string_new_with_size(c_strPtr, Runtime_c_str_length(c_strPtr));
}

Runtime_string_handle Runtime_string_new_with_size(const charT* c_strPtr, uint64T size)
{
	Runtime_string_handle result = nullptr;

	result = Runtime_string_new_empty();
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(result)->internalData;

	internalData->strData = Runtime_array_new(size+1, typeInteger8);
	auto strSize = Runtime_array_size(internalData->strData);
	charT* chPtr = nullptr;
	for (uint64T i = 0; i < strSize-1; i++) {
		chPtr = (charT*)Runtime_array_at(internalData->strData, i);
		*chPtr = c_strPtr[i];
	}
	chPtr = (charT*)Runtime_array_at(internalData->strData, strSize - 1);
	*chPtr = 0;

	return result;
}

Runtime_string_handle Runtime_string_new_char_count(charT ch, uint64T count)
{
	Runtime_string_handle result = nullptr;

	result = Runtime_string_new_empty();
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(result)->internalData;

	internalData->strData = Runtime_array_new(count + 1, typeInteger8);
	auto strSize = Runtime_array_size(internalData->strData);
	charT* chPtr = nullptr;
	for (uint64T i = 0; i < strSize - 1; i++) {
		chPtr = (charT*)Runtime_array_at(internalData->strData, i);
		*chPtr = ch;
	}
	chPtr = (charT*)Runtime_array_at(internalData->strData, strSize - 1);
	*chPtr = 0;

	return result;
}

Runtime_string_handle Runtime_string_new_copy(Runtime_string_handle rhs)
{
	Runtime_string_handle result = nullptr;
	result = Runtime_string_new_empty();
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(result)->internalData;

	Runtime_string_internal_data* rhsInternalData = RUNTIME_STRING_INTERNAL_DATA(rhs)->internalData;

	internalData->strData = Runtime_array_new_copy(rhsInternalData->strData);
	internalData->hashval = rhsInternalData->hashval;

	return result;
}

void Runtime_string_delete(Runtime_string_handle self)
{
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;
	Runtime_array_delete(internalData->strData);
	Runtime_free(self);
}

bool Runtime_string_empty(Runtime_string_handle self)
{
	return Runtime_string_size(self) == 0 ? true : false;
}

uint64T Runtime_string_size(Runtime_string_handle self)
{
	if (nullptr == self) {
		return 0;
	}
	
	return Runtime_array_size((RUNTIME_STRING_INTERNAL_DATA(self)->internalData)->strData);
}

uint64T Runtime_string_size_bytes(Runtime_string_handle self)
{ 
	uint64T result = 0;

	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;
	
	result = Runtime_array_size_bytes(internalData->strData);

	return result;
}

uint64T Runtime_string_hash(Runtime_string_handle self)
{
	uint64T result = 0;
	
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;
	if (0 == internalData->hashval) {
		
		auto chPtr = (byteT*)Runtime_array_at(internalData->strData, 0);
		auto sz = Runtime_array_size_bytes(internalData->strData);
		internalData->hashval = Runtime_hash_basic_bytes(chPtr, sz);
	}
	


	return internalData->hashval;
}

void Runtime_string_clear(Runtime_string_handle self)
{
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;
	Runtime_array_clear(internalData->strData);
	internalData->hashval = 0;
}

void Runtime_string_assign(Runtime_string_handle self, charT* c_strPtr)
{	
	Runtime_string_assign_with_size(self, c_strPtr, Runtime_c_str_length(c_strPtr));
}

void Runtime_string_assign_with_size(Runtime_string_handle self, charT* c_strPtr, uint64T size)
{
	Runtime_string_clear(self);
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;
	internalData->strData = Runtime_array_new(size + 1, typeInteger8);

	
	Runtime_heap_array* arrayData = RUNTIME_HEAP_ARRAY(internalData->strData);

	Runtime_mem_cpy(c_strPtr, arrayData->data, size * arrayData->infoPtr->stride);

	auto lastChPtr = (charT*)Runtime_array_at(internalData->strData, size);
	*lastChPtr = 0;

}

void Runtime_string_assign_char_count(Runtime_string_handle self, charT ch, uint64T count)
{
	Runtime_string_clear(self);
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;
	internalData->strData = Runtime_array_new(count + 1, typeInteger8);


	Runtime_heap_array* arrayData = RUNTIME_HEAP_ARRAY(internalData->strData);


	auto strSize = Runtime_array_size(internalData->strData);
	charT* chPtr = nullptr;
	for (uint64T i = 0; i < strSize - 1; i++) {
		chPtr = (charT*)Runtime_array_at(internalData->strData, i);
		*chPtr = ch;
	}
	chPtr = (charT*)Runtime_array_at(internalData->strData, strSize - 1);
	*chPtr = 0;
}

void Runtime_string_assign_copy(Runtime_string_handle self, Runtime_string_handle rhs)
{
	Runtime_string_clear(self);
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;

	Runtime_string_internal_data* rhsInternalData = RUNTIME_STRING_INTERNAL_DATA(rhs)->internalData;
	
	internalData->strData = Runtime_array_new_copy(rhsInternalData->strData);
}

charT Runtime_string_at(Runtime_string_handle self, uint64T index)
{
	charT result = 0;
	if (!Runtime_string_empty(self)) {
		auto valPtr = Runtime_array_at((RUNTIME_STRING_INTERNAL_DATA(self)->internalData)->strData, index);
		result = *((charT*)valPtr);
	}
	return result;
}

void Runtime_string_set(Runtime_string_handle self, uint64T index, charT ch)
{
	if (!Runtime_string_empty(self)) {
		auto valPtr = (charT*) Runtime_array_at((RUNTIME_STRING_INTERNAL_DATA(self)->internalData)->strData, index);
		*valPtr = ch;
	}
}


int32T Runtime_string_compare(Runtime_string_handle self, Runtime_string_handle rhs)
{
	int32T result = 0;
	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;

	Runtime_string_internal_data* rhsInternalData = RUNTIME_STRING_INTERNAL_DATA(rhs)->internalData;

	result = (int32T)Runtime_array_compare(internalData->strData, rhsInternalData->strData);

	return result;
}


Runtime_string_handle Runtime_string_substr(Runtime_string_handle self, uint64T start, uint64T count)
{
	Runtime_string_handle result = nullptr;

	if (nullptr == self) {
		return result;
	}

	Runtime_string_internal_data* internalData = RUNTIME_STRING_INTERNAL_DATA(self)->internalData;

	Runtime_heap_array* arrayData = RUNTIME_HEAP_ARRAY(internalData->strData);

	if ((start >= arrayData->size) || ((start + count) >= arrayData->size)) {
		return result;
	}

	



	return result;
}


uint64T Runtime_string_find(Runtime_string_handle self, Runtime_string_handle searchStr)
{
	return Runtime_NoIndx;
}

charT* Runtime_string_get_cstr(Runtime_string_handle self)
{
	return (charT*)Runtime_array_at((RUNTIME_STRING_INTERNAL_DATA(self)->internalData)->strData, 0);
}


//strings end
//----------------------------------------------------------------------------



bool Runtime_verify_heap_mem(void* mem)
{
	if (nullptr == mem) {
		return false;
	}

	uint8T* memPtr = (uint8T*)mem;
	memPtr -= sizeof(Runtime_memory_info);
	Runtime_memory_info* rtMemPtr = (Runtime_memory_info*)memPtr;
	
	if (mem != rtMemPtr->memPtr) {
		Runtime_debug_printf("Runtime_verify_heap_mem failed!\n");
	}

	return rtMemPtr->memPtr == mem;
}

void* Runtime_alloc(uint64T size, Runtime_TypeDescriptor type)
{
    void* result = nullptr;
	uint64T adjustedSize = 0;
	Runtime_memory_info memInfo;
	
	Runtime_memory_info_init(&memInfo);


	memInfo.size = size;
	memInfo.type = type;

	switch (type) {
		case typeRecord: {

		}break;

		case typeClass: {

		}break;

		case typeArray: {
			adjustedSize = size;
		}break;


		default: {
			adjustedSize = size;
		}break;
	}


	adjustedSize += sizeof(Runtime_memory_info);

	void* mem = Win32_Runtime_alloc(adjustedSize);

	//Runtime_debug_printf("Win32_Runtime_alloc: %d, mem: %p\n", (int)adjustedSize, mem);

	Runtime_memory_info* allocatedMem = (Runtime_memory_info*)mem;
	*allocatedMem = memInfo;
	uint8T* memPtr = (uint8T * )mem;
	allocatedMem->memPtr = (void*)(memPtr + sizeof(Runtime_memory_info));

	result = allocatedMem->memPtr;

	totalBytesHeapAllocated += adjustedSize;

	//Runtime_debug_printf("Runtime_alloc: %d, mem: %p\n", (int)allocatedMem->size, allocatedMem->memPtr);

	//Runtime_debug_printf("tot: %I64u\n", totalBytesHeapAllocated);

    return result;
}

void Runtime_free(void* mem)
{
	if (!Runtime_verify_heap_mem(mem)) {		
		return;
	}

	uint8T* memPtr = (uint8T*)mem;
	memPtr -= sizeof(Runtime_memory_info);

	Runtime_memory_info* allocatedMem = (Runtime_memory_info*)memPtr;
	//Runtime_debug_printf("Runtime_free: %d, Runtime_memory_info: %p, %p\n", (int)allocatedMem->size, allocatedMem, mem);

	totalBytesHeapAllocated -= allocatedMem->size + sizeof(Runtime_memory_info);

	Win32_Runtime_free(allocatedMem);
}

Runtime_memory_info_handle Runtime_RuntimeMemory_get_from_heap_ptr(void* mem)
{
	if (nullptr == mem) {
		return nullptr;
	}

	Runtime_memory_info* result = nullptr;
	uint8T* memPtr = (uint8T*)mem;
	memPtr -= sizeof(Runtime_memory_info);
	result = (Runtime_memory_info*)memPtr;

	if (result->memPtr != mem) {
		// this mem ptr is not something we created!
		result = nullptr;
	}

	return result;
}

uint64T Runtime_RuntimeMemory_get_size(void* mem)
{	
	if (nullptr == mem) {
		return 0;
	}
	auto tmp = (Runtime_memory_info*) Runtime_RuntimeMemory_get_from_heap_ptr(mem);
	if (nullptr == tmp) {
		return 0;
	}
		
	return tmp->size;
}

Runtime_TypeDescriptor Runtime_RuntimeMemory_get_type(void* mem)
{
	if (nullptr == mem) {
		return typeNilPtr;
	}

	auto tmp = (Runtime_memory_info*) Runtime_RuntimeMemory_get_from_heap_ptr(mem);
	if (nullptr == tmp) {
		return typeUnmanagedPtr;
	}

	return tmp->type;
}


//----------------------------------------------------------------------------
//hashtable

#define RUNTIME_HASHTABLE_NO_INDEX  uint64T (-1)
#define RUTNIME_HASHTABLE_DEF_CAPCITY_GROW	1.5
#define RUTNIME_HASHTABLE_DEF_MAX_USAGE		0.75

struct Runtime_hash_pair {
	void* keyPtr;
	void* valPtr;
	Runtime_hash_pair* next;
	Runtime_hash_pair* prev;
};

typedef Runtime_hash_pair** Runtime_hash_table_data_array;

struct Runtime_hashtable_object {
	Runtime_hash_table_data_array tableData;

	uint64T size;
	uint64T capacity;

	double capacityGrowthFactor;
	double maxUsageFactor;

	Runtime_hashtable_info* infoPtr;
};





uint64T Runtime_hashtable_hash_index(Runtime_hashtable_info* info, uint64T tableSize, void* key) {
	uint64T result = RUNTIME_HASHTABLE_NO_INDEX;
	
	switch (info->keyType) {
		case typeBit1: case typeBool:
		case typeInteger8: case typeUInteger8:
		case typeInteger16: case typeUInteger16:
		case typeInteger32: case typeUInteger32:
		case typeInteger64: case typeUInteger64:
		case typeInteger128: case typeUInteger128: {
			auto keyBytes = (uint8T*)key;
			result = Runtime_hash_basic_bytes(keyBytes, info->keyStride) % tableSize;

		}break;
		
		case typeDouble32: case typeDouble64: {
			auto keyBytes = (uint8T*)key;
			result = Runtime_hash_basic_bytes(keyBytes, info->keyStride) % tableSize;

		}break;

		case typeString:{
			auto str = (Runtime_string_handle)key;
			auto hashVal = Runtime_string_hash(str);
			result = hashVal % tableSize;

		}break;

		default: {
			
		} break;
	}

	RUNTIME_ASSERT(RUNTIME_HASHTABLE_NO_INDEX != result);

	return result;
}


Runtime_hash_pair* Runtime_hash_pair_new(Runtime_hashtable_info* info)
{
	Runtime_hash_pair* result = nullptr;
	//need a better way, this is probably absurdly inefficient
	result = (Runtime_hash_pair*)Runtime_alloc(sizeof(Runtime_hash_pair) + info->keyStride + info->valStride, typeUnknown);
	result->keyPtr = nullptr;
	result->valPtr = nullptr;
	result->next = nullptr;
	result->prev = nullptr;


	return result;
}

void Runtime_hash_pair_free_data(Runtime_hash_pair* self, Runtime_hashtable_info* info)
{
	switch (info->keyType) {
		case typeString: {
			//just a pointer, need to free up the string object
			Runtime_string_handle strPtr = self->keyPtr;
			Runtime_string_delete(strPtr);
		} break;

		case typeClass: {
			//just a pointer, need to free up the class object
			//Runtime_string_handle strPtr = self->keyPtr;
			//Runtime_string_delete(strPtr);
		} break;

		case typeArray: {
			//just a pointer, need to free up the class object
			Runtime_array_handle arrPtr = self->keyPtr;
			Runtime_array_delete(arrPtr);
		} break;

		default: {
			//no-op, data embedded in allocation
		} break;
	}

	switch (info->valueType) {
	case typeString: {
		//just a pointer, need to free up the string object
		Runtime_string_handle strPtr = self->valPtr;
		Runtime_string_delete(strPtr);
	} break;

	case typeClass: {
		//just a pointer, need to free up the class object
		//Runtime_string_handle strPtr = self->keyPtr;
		//Runtime_string_delete(strPtr);
	} break;

	case typeArray: {
		//just a pointer, need to free up the class object
		Runtime_array_handle arrPtr = self->valPtr;
		Runtime_array_delete(arrPtr);
	} break;

	case typeDictionary: {
		//just a pointer, need to free up the class object
		Runtime_dictionary_handle dictPtr = self->valPtr;
		Runtime_dictionary_delete(dictPtr);
	} break;

	default: {
		//no-op, data embedded in allocation
	} break;
	}

	Runtime_free(self);
}

void Runtime_hash_pair_free(Runtime_hash_pair* self, Runtime_hashtable_info* info)
{
	if (nullptr == self) {
		return;
	}

	auto nextP = self->next;

	Runtime_hash_pair_free_data(self, info);


	Runtime_hash_pair_free(nextP, info);
}

void Runtime_hash_pair_assign(Runtime_hash_pair* pair, Runtime_hashtable_info* info, void* key, void* val)
{
	if (nullptr == pair->keyPtr) {
		auto pairPtr = (uint8T*)pair;
		pair->keyPtr = (void*)(pairPtr + sizeof(Runtime_hash_pair));
	}
	if (nullptr == pair->valPtr) {
		auto pairPtr = (uint8T*)pair;
		pair->valPtr = (void*)(pairPtr + sizeof(Runtime_hash_pair) + info->keyStride);
	}	
	
	switch (info->keyType) {
		case typeString: {
			pair->keyPtr = (Runtime_string_handle)key;
		}break;

		case typeArray: {
			pair->keyPtr = (Runtime_array_handle)key;
		}break;

		case typeDictionary: {
			pair->keyPtr = (Runtime_dictionary_handle)key;
		}break;

		default: {
			Runtime_mem_cpy(key, pair->keyPtr, info->keyStride);
		}break;
	}

	switch (info->valueType) {
		case typeString: {
			pair->valPtr = (Runtime_string_handle)val;
		}break;

		case typeArray: {
			pair->valPtr = (Runtime_array_handle)val;
		}break;

		case typeDictionary: {
			pair->valPtr = (Runtime_dictionary_handle)val;
		}break;

		default: {
			Runtime_mem_cpy(val, pair->valPtr, info->valStride);
		}break;
	}
	
}


bool Runtime_hash_pair_equals(Runtime_hash_pair* pair, Runtime_hashtable_info* info, void* key) 
{	
	auto res = Runtime_mem_cmp(pair->keyPtr, info->keyStride, key, info->keyStride);
	if (res == 0) {
		return true;
	}
	return false;
}

Runtime_hash_pair* Runtime_hash_pair_find(Runtime_hash_pair* pair, Runtime_hashtable_info* info, void* key)
{
	Runtime_hash_pair* result = nullptr;
	auto np = pair->next;
	while (np != nullptr) {

		if (Runtime_hash_pair_equals(np, info, key)) {
			return np;
		}

		np = np->next;
	}
	return nullptr;
}


Runtime_hashtable_info* Runtime_get_hashtable_info_for_type(Runtime_TypeDescriptor keyType, Runtime_TypeDescriptor valType)
{
	Runtime_hashtable_info* result = nullptr;
	for (uint32T k = 0; k < runtimeInstancePtr->arrayInfoListSize; k++) {
		for (uint32T v = 0; v < runtimeInstancePtr->arrayInfoListSize; v++) {
			Runtime_hashtable_info* info = &runtimeInstancePtr->hashtableInfoList[k * runtimeInstancePtr->arrayInfoListSize + v];
			if (info->keyType == keyType && info->valueType == valType) {
				result = info;
				break;
			}
		}
	}

	return result;
}

Runtime_hashtable_handle Runtime_hashtable_new_struct(uint64T initialSize, Runtime_TypeDescriptor keyType, Runtime_TypeDescriptor valType)
{
	Runtime_hashtable_object* result = nullptr;

	auto hashTableInfo = Runtime_get_hashtable_info_for_type(keyType, valType);

	auto memPtr = Runtime_alloc(sizeof(Runtime_hashtable_object), typeDictionary);
	result = (Runtime_hashtable_object*)memPtr;
	
	uint64T allocSz = initialSize * (sizeof(Runtime_hash_pair*));// );
	result->tableData =  (Runtime_hash_pair**) Runtime_alloc(allocSz, typeUnknown);
	
	for (uint64T i = 0; i < initialSize;i++) {
		result->tableData[i] = nullptr;
	}

	result->capacity = initialSize;
	result->size = 0;
	result->infoPtr = hashTableInfo;
	result->capacityGrowthFactor = RUTNIME_HASHTABLE_DEF_CAPCITY_GROW;
	result->maxUsageFactor = RUTNIME_HASHTABLE_DEF_MAX_USAGE;

	return (Runtime_hashtable_handle)result;
}


#define DEFAULT_HASHTABLE_CAPACITY  10

Runtime_hashtable_handle Runtime_hashtable_new(Runtime_TypeDescriptor keyType, Runtime_TypeDescriptor valType)
{
	Runtime_hashtable_handle result = Runtime_hashtable_new_struct(DEFAULT_HASHTABLE_CAPACITY, keyType, valType);
	auto hashTable = (Runtime_hashtable_object*)result;

	return result;
}

void Runtime_hashtable_delete(Runtime_hashtable_handle self)
{
	auto hashTable = (Runtime_hashtable_object*)self;	

	Runtime_hashtable_clear(self);

	Runtime_free(hashTable->tableData);
	Runtime_free(self);
}


bool Runtime_hashtable_empty(Runtime_hashtable_handle self)
{
	auto hashTable = (Runtime_hashtable_object*)self;
	
	return 0 == hashTable->size;
}

uint64T Runtime_hashtable_size(Runtime_hashtable_handle self)
{
	auto hashTable = (Runtime_hashtable_object*)self;

	return hashTable->size;
}

uint64T Runtime_hashtable_capacity(Runtime_hashtable_handle self)
{
	auto hashTable = (Runtime_hashtable_object*)self;
	return hashTable->capacity;
}

double Runtime_hashtable_max_usage_factor(Runtime_hashtable_handle self)
{
	auto hashTable = (Runtime_hashtable_object*)self;
	return hashTable->maxUsageFactor;
}

void Runtime_hashtable_set_max_usage_factor(Runtime_hashtable_handle self, double val)
{
	auto hashTable = (Runtime_hashtable_object*)self;
	hashTable->maxUsageFactor = val;
}

double Runtime_hashtable_capacity_grow_by(Runtime_hashtable_handle self)
{
	auto hashTable = (Runtime_hashtable_object*)self;
	return hashTable->capacityGrowthFactor;
}

void Runtime_hashtable_set_capacity_grow_by(Runtime_hashtable_handle self, double val)
{
	auto hashTable = (Runtime_hashtable_object*)self;
	hashTable->capacityGrowthFactor = val;
}


void Runtime_hashtable_rebuild_hashes(
	Runtime_hash_table_data_array oldTableData, 
	Runtime_hash_table_data_array newTableData, 
	uint64T oldCapacity, 
	uint64T newCapacity,
	Runtime_hashtable_info* info)
{
	for (uint64T i = 0; i < oldCapacity; i++) {

		auto pair = oldTableData[i];



		if (nullptr != pair) {

			auto np = pair;
			while (nullptr != np) {
				auto idx = Runtime_hashtable_hash_index(info, newCapacity, np->keyPtr);
				auto nextP = np->next;

				auto existingPair = newTableData[idx];
				if (nullptr != existingPair) {
					auto np2 = existingPair;
					while (nullptr != np2) {
						existingPair = np2;
						np2 = np2->next;
					}
					np->next = nullptr;
					existingPair->next = np;
				}
				else {
					np->next = nullptr;
					newTableData[idx] = np;
				}

				np = nextP;
			}
		}
	}
}

void Runtime_hashtable_set_capacity(Runtime_hashtable_handle self, uint64T capacity)
{
	RUNTIME_ASSERT(capacity!=0);
	RUNTIME_ASSERT(nullptr != self);

	auto hashTable = (Runtime_hashtable_object*)self;
	auto oldCapacity = hashTable->capacity;
	hashTable->capacity = capacity;

	//Runtime_debug_printf("Runtime_hashtable_insert new cap: %I64u, size: %I64u\n", hashTable->capacity, hashTable->size);
	uint64T allocSz = hashTable->capacity * (sizeof(Runtime_hash_pair*));
	auto newTableData = (Runtime_hash_pair**)Runtime_alloc(allocSz, typeUnknown);
	Runtime_Memory_init(newTableData, allocSz);

	Runtime_hashtable_rebuild_hashes(hashTable->tableData, newTableData, oldCapacity, hashTable->capacity, hashTable->infoPtr);

	Runtime_free(hashTable->tableData);

	hashTable->tableData = newTableData;
}

void Runtime_hashtable_clear(Runtime_hashtable_handle self)
{
	auto hashTable = (Runtime_hashtable_object*)self;

	for (uint64T i = 0; i < hashTable->capacity; i++) {
		auto pair = hashTable->tableData[i];
		if (nullptr != pair) {
			Runtime_hash_pair_free(pair, hashTable->infoPtr);
		}

		hashTable->tableData[i] = nullptr;
	}
	hashTable->size = 0;
}

void Runtime_hashtable_insert(Runtime_hashtable_handle self, void* key, void* val)
{
	auto hashTable = (Runtime_hashtable_object*)self;

	uint64T tableMaxCap = ((double)hashTable->capacity) * hashTable->maxUsageFactor;
	if (hashTable->size >= tableMaxCap) {		
		Runtime_hashtable_set_capacity(self, (double)hashTable->capacity * hashTable->capacityGrowthFactor);
	}


	auto idx = Runtime_hashtable_hash_index(hashTable->infoPtr, hashTable->capacity, key);
	
	auto pair = hashTable->tableData[idx];

	if (pair == nullptr) {
		//first time
		pair = Runtime_hash_pair_new(hashTable->infoPtr);
		hashTable->tableData[idx] = pair;		
	}
	else {
		//find next
		auto np = pair;
		while (nullptr != np) {
			pair = np;
			np = np->next;
		}
		auto newPair = Runtime_hash_pair_new(hashTable->infoPtr);
		pair->next = newPair;
		pair = newPair;
	}


	Runtime_hash_pair_assign(pair, hashTable->infoPtr, key, val);

	hashTable->size++;
}

void Runtime_hashtable_erase(Runtime_hashtable_handle self, void* key)
{
	auto hashTable = (Runtime_hashtable_object*)self;
	auto idx = Runtime_hashtable_hash_index(hashTable->infoPtr, hashTable->capacity, key);
	
	auto pair = hashTable->tableData[idx];
	if (nullptr != pair) {

		auto nextP = pair->next;

		hashTable->tableData[idx] = nextP;

		Runtime_hash_pair_free(pair, hashTable->infoPtr);

		hashTable->size--;
	}
	
}

void* Runtime_hashtable_at(Runtime_hashtable_handle self, void* key)
{
	void* result = nullptr;
	auto hashTable = (Runtime_hashtable_object*)self;
	auto idx = Runtime_hashtable_hash_index(hashTable->infoPtr, hashTable->capacity, key);

	auto pair = hashTable->tableData[idx];
	if (nullptr != pair) {
		if (nullptr != pair->next) {
			pair = Runtime_hash_pair_find(pair, hashTable->infoPtr, key);
		}


		if (nullptr != pair) {
			result = pair->valPtr;
		}
	}
	

	return result;

}


//end of hashtable
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//dictionary

Runtime_dictionary_handle Runtime_dictionary_new(Runtime_TypeDescriptor valType)
{
	Runtime_dictionary_handle result = nullptr;


	result = Runtime_hashtable_new(typeString, valType);


	return result;
}

void Runtime_dictionary_delete(Runtime_dictionary_handle self)
{
	Runtime_hashtable_delete(self);
}


bool Runtime_dictionary_empty(Runtime_dictionary_handle self)
{
	return Runtime_hashtable_empty(self);
}

uint64T Runtime_dictionary_size(Runtime_dictionary_handle self)
{
	return Runtime_hashtable_size(self);
}

uint64T Runtime_dictionary_capacity(Runtime_dictionary_handle self)
{
	return Runtime_hashtable_capacity(self);
}

void Runtime_dictionary_set_capacity(Runtime_dictionary_handle self, uint64T capacity)
{
	Runtime_hashtable_set_capacity(self, capacity);
}

void Runtime_dictionary_clear(Runtime_dictionary_handle self)
{
	Runtime_hashtable_clear(self);
}

void Runtime_dictionary_insert(Runtime_dictionary_handle self, Runtime_string_handle key, void* val)
{
	Runtime_hashtable_insert(self, key, val);
}

void Runtime_dictionary_erase(Runtime_dictionary_handle self, Runtime_string_handle key)
{
	Runtime_hashtable_erase(self, key);
}



//[] access
void* Runtime_dictionary_at(Runtime_dictionary_handle self, Runtime_string_handle key)
{	
	return Runtime_hashtable_at(self, key);
}





//end of dictionary
//----------------------------------------------------------------------------


int32T Runtime_init()
{
	int result = 0;
	Runtime_debug_printf("Runtime_init\n");

	totalBytesHeapAllocated = 0;

	runtimeInstancePtr = (Runtime_Instance*) Runtime_alloc( sizeof(Runtime_Instance), typeUnknown);
	
	

	Runtime_TypeDescriptor types[] = {typeUnknown,
										typeRecord,
										typeClass,
										typeBit1,
										typeInteger8,
										typeUInteger8,
										typeInteger16,
										typeUInteger16,		
										typeInteger32,
										typeUInteger32,
										typeInteger64,
										typeUInteger64,
										typeInteger128,
										typeUInteger128,
										typeDouble32,
										typeDouble64,
										typeBool,
										typeString,
										typeArray,
										typeDictionary,
										typeMessage };

	uint32T typeCount = sizeof(types) / sizeof(types[0]);
	
	runtimeInstancePtr->initialArrayInfoSize = typeCount;
	runtimeInstancePtr->arrayInfoList = (Runtime_array_info*)Runtime_alloc(sizeof(Runtime_array_info) * runtimeInstancePtr->initialArrayInfoSize, typeUnknown);
	runtimeInstancePtr->arrayInfoListSize = runtimeInstancePtr->initialArrayInfoSize;

	
	runtimeInstancePtr->initialHashtableInfoSize = typeCount * typeCount;
	runtimeInstancePtr->hashtableInfoList = (Runtime_hashtable_info*)Runtime_alloc(sizeof(Runtime_hashtable_info) * runtimeInstancePtr->initialHashtableInfoSize, typeUnknown);
	runtimeInstancePtr->hashtableInfoListSize = runtimeInstancePtr->initialHashtableInfoSize;


	for (uint32T i = 0; i < runtimeInstancePtr->arrayInfoListSize;i++) {
		Runtime_array_info* arrInfo = &runtimeInstancePtr->arrayInfoList[i];
		Runtime_Memory_init(arrInfo, sizeof(Runtime_array_info));
		arrInfo->elementType = types[i];
		arrInfo->stride = Runtime_calc_stride_for_type(arrInfo->elementType);		
	}



	for (uint32T k = 0; k < typeCount; k++) {
		for (uint32T v = 0; v < typeCount; v++) {
			Runtime_hashtable_info* htInfo = &runtimeInstancePtr->hashtableInfoList[k* typeCount + v];
			Runtime_Memory_init(htInfo, sizeof(Runtime_hashtable_info));


			htInfo->keyType = types[k];
			htInfo->valueType = types[v];
			htInfo->keyStride = Runtime_calc_stride_for_type(htInfo->keyType);
			htInfo->valStride = Runtime_calc_stride_for_type(htInfo->valueType);
		}
	}

	return 1;
}

int32T internal_Runtime_main_entry_point(int32T argc, int8T** argv)
{
	int32T result = 0;
	
	Runtime_printf("Runtime_main_entry_point, argc: %d, argv: %p \n", argc, argv);
	for (int i = 0; i < argc; i++) {
		Runtime_printf("%d : argv: %s \n", i, argv[i]);
	}


	auto arr = Runtime_array_new(10, typeInteger32);
	auto arrSz = Runtime_array_size(arr);
	/*
	for (uint32T i = 0; i < arrSz;i++) {
		
		auto valPtr = Runtime_array_at(arr, i);

		int32T v = *((int32T*)valPtr);
		Runtime_printf("%d : v: %d, type: %d \n", i, v, (int)Runtime_array_type(arr) );
	}

	for (uint32T i = 0; i < arrSz; i++) {
		
		auto valPtr = (int32T * )Runtime_array_at(arr, i);
		*valPtr = i * 25;		
	}


	for (uint32T i = 0; i < arrSz; i++) {
		
		auto valPtr = Runtime_array_at(arr, i);
		int32T v = *((int32T*)valPtr);
		Runtime_printf("%d : v: %d, type: %d \n", i, v, (int)Runtime_array_type(arr));
	}
	*/
	Runtime_array_delete(arr);


	auto str = Runtime_string_new("Hello World");
	Runtime_printf("str(%p): %s, char count: %I64u, size bytes: %I64u\n", str, Runtime_string_get_cstr(str), Runtime_string_size(str), Runtime_string_size_bytes(str));
	Runtime_printf("hash: %I64u\n", Runtime_string_hash(str));

	auto str2 = Runtime_string_new("Hello World");


	auto res = Runtime_string_compare(str, str2);

	Runtime_printf("cmp: %d\n", res);
	Runtime_string_delete(str2);

	str2 = Runtime_string_new("Foobar");
	Runtime_printf("str(%p): %s, char count: %I64u, size bytes: %I64u\n", str2, Runtime_string_get_cstr(str2), Runtime_string_size(str2), Runtime_string_size_bytes(str2));


	res = Runtime_string_compare(str, str2);
	Runtime_printf("cmp: %d\n", res);


	Runtime_string_delete(str);
	Runtime_string_delete(str2);



	arr = Runtime_array_new(10, typeFloat);
	/*
	for (int i = 0; i < Runtime_array_size(arr); i++) {
		float* f = (float*)Runtime_array_at(arr, i);
		*f = 2.443f;

		int ip = (int)*f;
		float dp = *f - (float)ip;
		dp *= 1000.0;
		int dip = (int)dp;

		Runtime_printf("%d : f: %d.%d \n", i, ip, dip);
	}
	*/
	Runtime_printf("%d : %p \n", 10, Runtime_array_at(arr,10));

	float f = 1.0;
	for (int i = 0; i < 1000; i++) {		
				
		
		Runtime_array_append(arr, &f);
		f += 0.015f;
		
	}
	
	/*
	for (int i = 0; i < Runtime_array_size(arr); i++) {
		float f = *((float*)Runtime_array_at(arr, i));
		int ip = (int)f;
		float dp = f - (float)ip;
		dp *= 1000.0;
		int dip = (int)dp;

		Runtime_printf("%d : f: %d.%d \n", i, ip,dip);
	}
	*/
	Runtime_array_delete(arr);

	return result;

}
int32T Runtime_main_entry_point()
{
	int32T argc = 0;
	int8T** argv = Win32_get_command_line(&argc);

	auto result = internal_Runtime_main_entry_point(argc,argv);
	
	Win32_Runtime_free(argv);

	return result;
}

//termination
void Runtime_terminate()
{
	Runtime_debug_printf("Runtime_terminate\n");

	Runtime_free(runtimeInstancePtr->arrayInfoList);
	Runtime_free(runtimeInstancePtr->hashtableInfoList);
	Runtime_free(runtimeInstancePtr);
	runtimeInstancePtr = nullptr;

	
	Runtime_debug_printf("bytes leftover : %I64u\n", totalBytesHeapAllocated);

	Runtime_debug_printf("Runtime_terminate finished\n");
}