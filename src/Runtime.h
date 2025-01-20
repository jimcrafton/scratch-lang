#pragma once


#include <vector>
#include <map>
#include <unordered_map>
#include <string>



namespace language {

	namespace runtime {
		class Runtime {
		public:
			static void init();
			static void finish();




		private:

			Runtime() {

			}

			~Runtime() {

			}

			static Runtime* rtInstance;
		};
	}
}