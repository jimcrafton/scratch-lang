#pragma once

#include "types.h"
#include <string>



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
	private:
		void* data = nullptr;
	};



	class object {

	};

}
