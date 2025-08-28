#include "scratch_tool.h"


utils::cmd_line_options::option_array_t scratch_tool::opts = {
	utils::cmd_line_options::action_option("",compiler::CompilerOptions::COMP_OPT_HELP, "how to use..."),
	utils::cmd_line_options::action_option("",compiler::CompilerOptions::COMP_OPT_VERSION, "display version information"),
	utils::cmd_line_options::action_option("",compiler::CompilerOptions::COMP_OPT_PRINT_AST, "prints out ast for translation units only"),
	utils::cmd_line_options::action_option("",compiler::CompilerOptions::COMP_OPT_PRINT_CODE_OBJ_MODEL, "prints out code object model"),

	utils::cmd_line_options::bool_option("",compiler::CompilerOptions::COMP_OPT_NO_LOGO, false, "hide logo for unit compilation"),
	utils::cmd_line_options::bool_option("",compiler::CompilerOptions::COMP_OPT_VERBOSE_MODE, false, "enable/disable verbose mode for extra reporting"),
	utils::cmd_line_options::bool_option("",compiler::CompilerOptions::COMP_OPT_DEBUG_MODE, false, "enable/disable debug mode for even more reporting"),
	utils::cmd_line_options::bool_option("",compiler::CompilerOptions::COMP_OPT_COMPILE_ONLY,false, "only compiles input files, no linking attempted")
};


