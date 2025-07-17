#pragma once

#ifndef _TOKEN_H__
#define _TOKEN_H__

#include <vector>
#include <map>

#include "string_ref.h"


namespace language {
    enum KeywordIdx {
        KEYWORD_IF = 0,
        KEYWORD_ELSE,
        KEYWORD_TRUE,
        KEYWORD_FALSE,
        KEYWORD_MODULE,
        KEYWORD_PROGRAM,
        KEYWORD_LIB,
        KEYWORD_IMPORT,
        KEYWORD_NAMESPACE,
        KEYWORD_CLASS,
        KEYWORD_RECORD,
        KEYWORD_INHERITS,
        KEYWORD_IMPLEMENTS,
        KEYWORD_PRIVATE,
        KEYWORD_PUBLIC,
        KEYWORD_STATIC,
        KEYWORD_RETURN,
        KEYWORD_NIL,
        KEYWORD_SELF,
        KEYWORD_SUPER,
        KEYWORD_MSG
    };
    const std::vector<std::string> Keywords = {
            "if",
            "else",
            "true",
            "false",
            "module",
            "program",
            "lib",
            "import",
            "namespace",
            "class",
            "record",
            "inherits",
            "implements",
            "private",
            "public",
            "static",
            "return",
            "nil",
            "self",
            "super",
            "msg"
    };

    enum ReservedCharactersIdx {
        RESERVED_CH_OPEN_PAREN = 0,
        RESERVED_CH_CLOSE_PAREN,
        RESERVED_CH_OPEN_BRACKET,
        RESERVED_CH_CLOSE_BRACKET,
        RESERVED_CH_OPEN_BLOCK,
        RESERVED_CH_CLOSE_BLOCK,
        RESERVED_CH_END_OF_STATEMENT,
        RESERVED_CH_ADDITION_OP,
        RESERVED_CH_SUBTRACTION_OP,
        RESERVED_CH_DIVISION_OP,
        RESERVED_CH_MULTIPLICATION_OP,
        RESERVED_CH_COLON,
        RESERVED_CH_AT_SIGN,
        RESERVED_CH_DOT,

    };
    const std::string ReservedCharacters = "()[]{};+-/*:@.";
}

namespace lexer {

    class CodeLocation {
    public:
        size_t lineNumber=0;
        size_t colNumber=0;
        size_t offset=0;
        std::string filename;


        bool valid() const {
            return !(lineNumber == 0 && colNumber == 0 && offset == 0);
        }
    };


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

        CodeLocation location;

        string_ref text;
        Type type= UNKNOWN;

        Token() {}

        bool valid() const {
            return location.valid() && type != UNKNOWN;
        }

        bool isMathOperator() const {
            return type == ADDITION_OPERATOR || type == SUBTRACTION_OPERATOR ||
                type == MULT_OPERATOR || type == DIV_OPERATOR || type == MOD_OPERATOR;
        }

        bool isOperator() const {
            return isMathOperator() || type == ASSIGMENT_OPERATOR; //other ops like <, >, =/equality, etc
        }

        bool isLiteral() const {
            return type == INTEGER_LITERAL || type == DECIMAL_LITERAL ||
                type == HEXADECIMAL_LITERAL || type == BINARY_LITERAL ||
                type == BOOLEAN_LITERAL || type == STRING_LITERAL;
        }

        bool isNil() const {
            return text == language::Keywords[language::KEYWORD_NIL];
        }

        static bool no_case_cmp(std::string::value_type lhs, std::string::value_type rhs) {
            return std::tolower(lhs) == std::tolower(rhs);
        }
        //case insensitive compare because we could care less!!!
        bool operator==(const std::string& rhs) const {
            std::string lhs = text.str();

            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), Token::no_case_cmp);
        }

        std::string info() const {
            std::string result;
            if (valid()) {
                result = lexer::Token::tokenNames.find(this->type)->second;
                result += " : '" + text.str() + "'";
            }
            
            return result;
        }
    };

}


#endif //_TOKEN_H__
