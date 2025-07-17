#pragma once



#include "Lexer.h"
#include "Parser.h"

#include "compiler.h"
#include "cmd_line_options.h"


class scratch_tool {
public:
	static utils::cmd_line_options::option_array_t opts;

	static int main(int argc, char** argv);

	typedef std::map<std::string, std::string> ArgMapT;

	static int main(const ArgMapT& args, const std::vector<std::string>& files);

};