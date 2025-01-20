// new-lang-test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <map>


#include "Lexer.h"
#include "Parser.h"



int main(int argc, char** argv)
{
    std::string filename = "tests/test3.scratch";

    Lexer lexer;

    
    try {
        lexer.lex(filename);
    }
    catch (const Lexer::Error& e) {
        e.output(lexer);
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

    
    for (const Token& t : lexer.currentContext()->tokens) {
        std::cout << Token::tokenNames.find(t.type)->second << ": '" << t.text.str() << "'" << std::endl;
    }


    Parser parser;

    try {
        parser.parse(lexer);

        s = "";
        s.append(80, '*');
        std::cout << "Printing AST" << std::endl;
        std::cout << s << std::endl;
        parser.ast.print();
        std::cout << s << std::endl;
    }
    catch (const Parser::Error& e) {
        s = "";
        s.append(80, '*');
        std::cout << s << std::endl;
        e.output();
        std::cout << s << std::endl;
        return -1;
    }
    
    

    return 0;
}

