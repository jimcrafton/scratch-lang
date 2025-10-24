#pragma once

#include "types.h"
#include <string>

#include "scratch_runtime.h"



namespace runtime {
	class Runtime;
}

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

		void assign(const ::std::string& v);

		void assign(const charT* c_strPtr);		

		uint64_t length() const;
		uint64_t empty() const;

		const char* c_str() const { return data.c_str(); }
	private:
		std::string data;
	};



	class object {
		


	};

}
