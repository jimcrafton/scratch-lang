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

    
    
    if (!lexer.lex(filename)) {
        lexer.getError().output(lexer);
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

    std::map<Token::Type, std::string> ttmap;
    ttmap = {
        {Token::UNKNOWN,"UNKNOWN" },
        {Token::INTEGER_LITERAL,"INTEGER_LITERAL"},
        {Token::STRING_LITERAL,"STRING_LITERAL"},
        {Token::DECIMAL_LITERAL,"DECIMAL_LITERAL"},
        {Token::HEXADECIMAL_LITERAL,"HEXADECIMAL_LITERAL"},
        {Token::BINARY_LITERAL,"BINARY_LITERAL"},
        {Token::END_OF_STATEMENT,"END_OF_STATEMENT"},
        {Token::ASSIGMENT_OPERATOR,"ASSIGMENT_OPERATOR"},
        {Token::ADDITION_OPERATOR,"ADDITION_OPERATOR"},
        {Token::SUBTRACTION_OPERATOR,"SUBTRACTION_OPERATOR"},
        {Token::MULT_OPERATOR,"MULT_OPERATOR"},
        {Token::DIV_OPERATOR,"DIV_OPERATOR"},
        {Token::MOD_OPERATOR,"MOD_OPERATOR"},
        {Token::IDENTIFIER,"IDENTIFIER"},
        {Token::BOOLEAN_LITERAL,"BOOLEAN_LITERAL"},
        {Token::KEYWORD,"KEYWORD"},        
        {Token::OPEN_PAREN,"OPEN_PAREN"},
        {Token::CLOSE_PAREN,"CLOSE_PAREN"},
        {Token::OPEN_BLOCK,"OPEN_BLOCK"},
        {Token::CLOSE_BLOCK,"CLOSE_BLOCK"},
        {Token::COMMENT,"COMMENT"},
        {Token::COMMENT_START,"COMMENT_START"},
        {Token::COMMENT_END,"COMMENT_END"},
        {Token::COMMA,"COMMA"},
        {Token::COLON,"COLON"},
        {Token::EQUALS_SIGN,"EQUALS_SIGN"},
        
        {Token::VERSION_LITERAL,"VERSION_LITERAL"}
    };
    for (const Token& t : lexer.currentContext()->tokens) {
        std::cout << ttmap[t.type] << ": '" << t.text.str() << "'" << std::endl;
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
    catch (const ParseError& e) {
        s = "";
        s.append(80, '*');
        std::cout << s << std::endl;
        e.output();
        std::cout << s << std::endl;
    }
    
    

    return 0;
}

