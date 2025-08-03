// scratch_lang_runtime_test_harness.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "scratch_runtime.h"
#include <Windows.h>



class HiResTimer {
private:
	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;
	LARGE_INTEGER freq;

	double duration = 0.0;
public:	

	void start() {
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&startTime);
		duration = 0;
	}

	void stop() {
		QueryPerformanceCounter(&endTime); 
		LARGE_INTEGER elapsed;

		elapsed.QuadPart = endTime.QuadPart - startTime.QuadPart;
		duration = (double)(elapsed.QuadPart*1000) / (double)freq.QuadPart;
	}

	operator double() const {
		return duration;
	}

};


class Decimal {
	double d=0.0;
public:
	Decimal() {}
	Decimal(double v):d(v) {}

	Decimal& operator=(double v) {
		d = v;
		return *this;
	}

	int32T intpart() const {
		return (int32T)d;
	}
	int32T decimal(int ndr) const {
		auto tmp = (d - (double)intpart());
		for (int i = 0; i < ndr; i++) {
			tmp *= 10.0;
		}
		return (int32T)tmp;
	}
};


extern uint64T Runtime_mem_cpy(void* src, void* dest, uint64T size);

int main()
{
	Runtime_init();


	Runtime_printf("*********************************************************************\n");

	HiResTimer t;
	t.start();

	auto arr = Runtime_array_new(10, typeFloat);
	Runtime_printf("%d : %p \n", 10, Runtime_array_at(arr, 2));
	
	Runtime_array_delete(arr);

	auto ht = Runtime_hashtable_new(typeInteger32, typeInteger32);
	int32T k = 10;
	int32T v = 984568589;
	Runtime_hashtable_insert(ht, &k, &v);
	
	k = 5;
	v = 574743;
	Runtime_hashtable_insert(ht, &k, &v);
	t.stop();
	Decimal dur(t);

	Runtime_printf("took: %d.%d ms\n", dur.intpart(), dur.decimal(3));

	auto val = Runtime_hashtable_at(ht, &k);
	Runtime_printf("k: %d, v: %d \n", k, *((int32T*)val) );

	t.start();
	for (int i = 0; i < 2000;i++) {
		k = i * 3 + 1;
		v = 7896 + k;

		if (k == 703) {
			Runtime_printf("hit k = 703\n");
		}

		Runtime_hashtable_insert(ht, &k, &v);
	}

	t.stop();
	dur = t;
	Runtime_printf("Runtime_hashtable_insert took: %d.%d ms for %d \n", dur.intpart(), dur.decimal(3), (int)Runtime_hashtable_size(ht));

	k = 234 * 3 + 1;

	t.start();
	val = Runtime_hashtable_at(ht, &k);
	t.stop();

	Runtime_printf("k: %d, v: %d \n", k, *((int32T*)val));
	
	dur = t;
	Runtime_printf("took: %d.%d ms\n", dur.intpart(), dur.decimal(3));

	Runtime_hashtable_erase(ht, &k);

	t.start();
	val = Runtime_hashtable_at(ht, &k);
	t.stop();

	Runtime_printf("found k: %s \n", val==nullptr ? "false":"true");


	t.start();
	Runtime_hashtable_delete(ht);
	t.stop();
	dur = t;
	Runtime_printf("Runtime_hashtable_delete took: %d.%d ms\n", dur.intpart(), dur.decimal(3));



	auto strK1 = Runtime_string_new("Number1");

	auto dict = Runtime_dictionary_new(typeInteger32);
	v = 100;
	Runtime_dictionary_insert(dict, strK1, &v);

	auto foundVal = Runtime_dictionary_at(dict, strK1);

	Runtime_printf("found k: %s val: %d \n", Runtime_string_get_cstr(strK1), foundVal? *((int32T*)foundVal):-1 );

	Runtime_dictionary_delete(dict);


	strK1 = Runtime_string_new("Number1");
	auto strV1 = Runtime_string_new("Hello World");
	dict = Runtime_dictionary_new(typeString);
	Runtime_dictionary_insert(dict, strK1, strV1);

	foundVal = Runtime_dictionary_at(dict, strK1);

	Runtime_printf("found k: %s val: %s \n", Runtime_string_get_cstr(strK1), foundVal ? Runtime_string_get_cstr(foundVal) : "null");


	auto strK2 = Runtime_string_new("Junk");
	foundVal = Runtime_dictionary_at(dict, strK2);

	Runtime_printf("found k: %s val: %s \n", Runtime_string_get_cstr(strK2), foundVal ? Runtime_string_get_cstr(foundVal) : "null");


	Runtime_dictionary_delete(dict);


	Runtime_printf("k2 size bytes: %d\n", (int)Runtime_string_size_bytes(strK2));
	Runtime_string_delete(strK2);
	


	int intArr[] = {23,44,55,988,1209};


	int idx = 3;
	auto stackArr = Runtime_array_new_from_stack(intArr, sizeof(intArr) / sizeof(intArr[0]), typeInteger32);

	Runtime_printf("%d : %d:[%p] %d\n", Runtime_array_size(stackArr), idx, Runtime_array_at(stackArr, idx), *((int*)Runtime_array_at(stackArr, idx)));

	Runtime_array_delete(stackArr);


	Runtime_printf("*********************************************************************\n");

	Runtime_terminate();


	return 0;
}
