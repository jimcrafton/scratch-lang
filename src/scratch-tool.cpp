
#include "scratch_tool.h"


utils::cmd_line_options::option_array_t scratch_tool::opts = {
	utils::cmd_line_options::action_option("",language::compiler::CompilerOptions::COMP_OPT_HELP, "how to use..."),
	utils::cmd_line_options::action_option("",language::compiler::CompilerOptions::COMP_OPT_VERSION, "display version information"),
	utils::cmd_line_options::action_option("",language::compiler::CompilerOptions::COMP_OPT_PRINT_AST, "prints out ast for translation units only"),
	
	utils::cmd_line_options::bool_option("",language::compiler::CompilerOptions::COMP_OPT_NO_LOGO, false, "hide logo for unit compilation"),
	utils::cmd_line_options::bool_option("",language::compiler::CompilerOptions::COMP_OPT_VERBOSE_MODE, false, "enable/disable verbose mode for extra reporting"),
	utils::cmd_line_options::bool_option("",language::compiler::CompilerOptions::COMP_OPT_DEBUG_MODE, false, "enable/disable debug mode for even more reporting"),
	utils::cmd_line_options::bool_option("",language::compiler::CompilerOptions::COMP_OPT_COMPILE_ONLY,false, "only compiles input files, no linking attempted")
};


int scratch_main(utils::cmd_line_options& cmdline)
{
	int returnCode = 0;

	auto files = cmdline.trailing_values();

	if (cmdline["help"]) {
		cmdline.usage();
		return 0;
	}

	if (cmdline["version"]) {
		std::cout << cmdline.get_app_name() << " version " << language::compiler::Compiler::version() << std::endl;
		return 0;
	}

	language::compiler::Compiler compiler(cmdline);
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