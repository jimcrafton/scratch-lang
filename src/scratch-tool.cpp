

#include "Lexer.h"
#include "Parser.h"

#include "compiler.h"
#include "cmd_line_options.h"

utils::cmd_line_options::option opts[] = {
	utils::cmd_line_options::action_option("","help", "how to use..."),
	utils::cmd_line_options::bool_option("","compile-only",false, "only compiles input files, no linking attempted"),
	utils::cmd_line_options::option::null()
};


int main(int argc, char** argv)
{
	int returnCode = 0;

	utils::cmd_line_options cmdline("scratch-tool", opts, sizeof(opts) / sizeof(opts[0]));
	if (!cmdline.parse(argc, argv)) {
		cmdline.usage();
		return - 1;
	}

	auto files = cmdline.trailing_values();

	

	if (cmdline["help"]) {
		cmdline.usage();
		return 0;
	}

	language::compiler::CompilerOptions opts;
	opts.init(cmdline);

	try {

		language::compiler::Compiler compiler;
		compiler.options = opts;

		compiler.build(files.vals());
	}
	catch (const Lexer::Error& e) {
		e.output();
		returnCode = -1;
	}
	catch (const Parser::Error& e) {
		e.output();
		returnCode = -1;
	}
	catch (const language::compiler::Compiler::Error& e) {
		e.output();
		returnCode = -1;
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		returnCode = -1;
	}


	return returnCode;
}