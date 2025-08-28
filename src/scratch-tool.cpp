
#include "scratch_tool.h"


int scratch_main(utils::cmd_line_options& cmdline)
{
	int returnCode = 0;

	auto files = cmdline.trailing_values();

	if (cmdline["help"]) {
		cmdline.usage();
		return 0;
	}

	if (cmdline["version"]) {
		std::cout << cmdline.get_app_name() << " version " << compiler::Compiler::version() << std::endl;
		return 0;
	}

	compiler::Compiler compiler(cmdline);
	try {
		compiler.build(files.vals());
	}
	catch (const lexer::Lexer::Error& e) {
		e.output();
		returnCode = -1;
	}
	catch (const parser::Parser::Error& e) {
		e.output();
		returnCode = -1;
	}
	catch (const compiler::Compiler::Error& e) {
		e.output();
		returnCode = -1;
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		returnCode = -1;
	}

	return returnCode;
}

int scratch_tool::main(const scratch_tool::ArgMapT& args, const std::vector<std::string>& files)
{
	utils::cmd_line_options cmdline("scratch-tool", opts);

	if (!cmdline.parse(args, files)) {
		cmdline.usage();
		return -1;
	}

	
	return scratch_main(cmdline);
}
 
int scratch_tool::main(int argc, char** argv)
{
	

	utils::cmd_line_options cmdline("scratch-tool", opts);
	if (!cmdline.parse(argc, argv)) {
		cmdline.usage();
		return -1;
	}


	return scratch_main(cmdline);
}