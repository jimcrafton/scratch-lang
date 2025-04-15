#pragma once

#ifndef _TOKEN_H__
#define _TOKEN_H__

#include <map>

#include "string_ref.h"

namespace lexer {

    class Token {
    public:
        enum Type {
            UNKNOWN = 0,
            INTEGER_LITERAL,
            DECIMAL_LITERAL,
            HEXADECIMAL_LITERAL,
            BINARY_LITERAL,
            BOOLEAN_LITERAL,
            STRING_LITERAL,
            END_OF_STATEMENT,
            ASSIGMENT_OPERATOR,
            ADDITION_OPERATOR,
            SUBTRACTION_OPERATOR,
            MULT_OPERATOR,
            DIV_OPERATOR,
            MOD_OPERATOR,
            IDENTIFIER,
            KEYWORD,
            OPEN_PAREN,
            CLOSE_PAREN,
            OPEN_BLOCK,
            CLOSE_BLOCK,
            OPEN_BRACKET,
            CLOSE_BRACKET,
            COMMENT,
            COMMENT_START,
            COMMENT_END,
            COMMA,
            COLON,
            EQUALS_SIGN,
            VERSION_LITERAL,
            AT_SIGN,
            DOT,
        };

        static const std::map< Token::Type, std::string> tokenNames;

        size_t lineNumber;
        size_t colNumber;
        size_t offset;

        std::string filename;

        string_ref text;
        Type type;

        Token() :lineNumber(0), colNumber(0), offset(0), type(UNKNOWN) {}

        bool valid() const {
            return !(lineNumber == 0 && colNumber == 0 && offset == 0 && type == UNKNOWN);
        }

        bool isMathOperator() const {
            return type == ADDITION_OPERATOR || type == SUBTRACTION_OPERATOR ||
                type == MULT_OPERATOR || type == DIV_OPERATOR || type == MOD_OPERATOR;
        }

        bool isLiteral() const {
            return type == INTEGER_LITERAL || type == DECIMAL_LITERAL ||
                type == HEXADECIMAL_LITERAL || type == BINARY_LITERAL ||
                type == BOOLEAN_LITERAL || type == STRING_LITERAL;
        }

        static bool no_case_cmp(std::string::value_type lhs, std::string::value_type rhs) {
            return std::tolower(lhs) == std::tolower(rhs);
        }
        //case insensitive compare because we could care less!!!
        bool operator==(const std::string& rhs) const {
            std::string lhs = text.str();

            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), Token::no_case_cmp);
        }
    };

}


#endif //_TOKEN_H__
