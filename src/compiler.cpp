#include "Compiler.h"

namespace language {
	namespace compiler {
		Compiler* Compiler::compilerInstance = NULL;

		void Compiler::init()
		{
			Compiler::compilerInstance = new Compiler();
		}

		void Compiler::finish()
		{
			delete Compiler::compilerInstance;
		}
	}
	
}
