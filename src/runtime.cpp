

#include "Runtime.h"



namespace language {

	namespace runtime {


		Runtime* Runtime::rtInstance = NULL;

		void Runtime::init()
		{
			Runtime::rtInstance = new Runtime();
		}

		void Runtime::finish()
		{
			delete Runtime::rtInstance;
		}
		
	}
}