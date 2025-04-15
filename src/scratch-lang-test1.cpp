// new-lang-test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include "Lexer.h"
#include "Parser.h"

#include "compiler.h"



int main(int argc, char** argv)
{
    std::string filename = "tests/test5.scratch";

    lexer::Lexer lexer;

    
    try {
        lexer.lex(filename);
    }
    catch (const lexer::Lexer::Error& e) {
        e.output();
        return -1;
    }
    

    std::string s = "";
    s.append(80, '-');
    std::cout << s << std::endl;
    std::cout << lexer.currentContext()->text << std::endl;
    std::cout << s << std::endl;


    s = "";
    std::cout << "Output tokens" << std::endl;
    s.append(80, '*');
    std::cout << s << std::endl;

    
    for (const lexer::Token& t : lexer.currentContext()->tokens) {
        std::cout << lexer::Token::tokenNames.find(t.type)->second << ": '" << t.text.str() << "'" << std::endl;
    }


    parser::Parser parser;

    try {
        parser.parse(lexer);

        s = "";
        s.append(80, '*');
        std::cout << "Printing AST" << std::endl;
        std::cout << s << std::endl;
        parser.ast.print();
        std::cout << s << std::endl;


    }
    catch (const parser::Parser::Error& e) {
        s = "";
        s.append(80, '*');
        std::cout << s << std::endl;
        e.output();
        std::cout << s << std::endl;
        return -1;
    }
    



    language::compiler::Compiler compiler;

    try {

        compiler.compile(parser.ast);

        compiler.dumpCompilerOutput();

        compiler.dumpCompilerObjectCode();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

