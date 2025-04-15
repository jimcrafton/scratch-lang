#include "Token.h"



const std::map< lexer::Token::Type, std::string> lexer::Token::tokenNames = {
        { lexer::Token::UNKNOWN, "UNKNOWN" },
        { lexer::Token::INTEGER_LITERAL, "INTEGER_LITERAL" },
        { lexer::Token::DECIMAL_LITERAL, "DECIMAL_LITERAL" },
        { lexer::Token::HEXADECIMAL_LITERAL, "HEXADECIMAL_LITERAL" },
        { lexer::Token::BINARY_LITERAL, "BINARY_LITERAL" },
        { lexer::Token::BOOLEAN_LITERAL, "BOOLEAN_LITERAL" },
        { lexer::Token::STRING_LITERAL, "STRING_LITERAL" },
        { lexer::Token::END_OF_STATEMENT, "END_OF_STATEMENT" },
        { lexer::Token::ASSIGMENT_OPERATOR, "ASSIGMENT_OPERATOR" },
        { lexer::Token::ADDITION_OPERATOR, "ADDITION_OPERATOR" },
        { lexer::Token::SUBTRACTION_OPERATOR, "SUBTRACTION_OPERATOR" },
        { lexer::Token::MULT_OPERATOR, "MULT_OPERATOR" },
        { lexer::Token::DIV_OPERATOR, "DIV_OPERATOR" },
        { lexer::Token::MOD_OPERATOR, "MOD_OPERATOR" },
        { lexer::Token::IDENTIFIER, "IDENTIFIER" },
        { lexer::Token::KEYWORD, "KEYWORD" },
        { lexer::Token::OPEN_PAREN, "OPEN_PAREN" },
        { lexer::Token::CLOSE_PAREN, "CLOSE_PAREN" },
        { lexer::Token::OPEN_BLOCK, "OPEN_BLOCK" },
        { lexer::Token::CLOSE_BLOCK, "CLOSE_BLOCK" },
        { lexer::Token::OPEN_BRACKET, "OPEN_BRACKET" },
        { lexer::Token::CLOSE_BRACKET, "CLOSE_BRACKET" },
        { lexer::Token::COMMENT, "COMMENT" },
        { lexer::Token::COMMENT_START, "COMMENT_START" },
        { lexer::Token::COMMENT_END, "COMMENT_END" },
        { lexer::Token::COMMA, "COMMA" },
        { lexer::Token::COLON, "COLON" },
        { lexer::Token::EQUALS_SIGN, "EQUALS_SIGN" },
        { lexer::Token::VERSION_LITERAL, "VERSION_LITERAL", },
        { lexer::Token::AT_SIGN, "AT_SIGN", },
        { lexer::Token::DOT, "DOT", },
};