// new-lang-test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <map>


#include "Lexer.h"




int main(int argc, char** argv)
{
    std::string fileText = "";

    FILE* f = fopen("test.scratch", "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        size_t fs = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* tmp = new char[fs+1];
        memset(tmp, 0, fs + 1);
        fread(tmp, 1, fs, f);
        fileText = tmp;
        delete[] tmp;
        fclose(f);
    }

    Lexer p;

    std::string s = "";
    s.append(80, '-');
    std::cout << s << std::endl;
    std::cout << fileText << std::endl;
    std::cout << s << std::endl;
    
    if (!p.process(fileText)) {
        std::cout << "Error at line: " << p.currentLine << std::endl;
    }


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
        {Token::MESSAGE_SIGNATURE,"MESSAGE_SIGNATURE"},
        {Token::OPEN_PAREN,"OPEN_PAREN"},
        {Token::CLOSE_PAREN,"CLOSE_PAREN"},
        {Token::OPEN_BLOCK,"OPEN_BLOCK"},
        {Token::CLOSE_BLOCK,"CLOSE_BLOCK"},
        {Token::COMMENT,"COMMENT"},
        {Token::COMMENT_START,"COMMENT_START"},
        {Token::COMMENT_END,"COMMENT_END"}
    };
    for (const Token& t : p.tokens) {
        std::cout << ttmap[t.type] << ": '" << t.text.str() << "'" << std::endl;
    }
    return 0;
}

