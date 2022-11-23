#ifndef _LEXER_H__
#define _LEXER_H__

#pragma once

#include "string_ref.h"
#include "Token.h"



class FileContext {
public:
    std::string fileName;    
    std::vector<Token> tokens;
    std::vector<size_t> newLines;
    std::string text;

    void clear() {
        tokens.clear();
        newLines.clear();
        text = "";
    }
};

class Lexer {

public:
    bool whitespace(const char& ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    bool digit(const char& ch)
    {
        return ch >= '0' && ch <= '9';
    }

    bool binarydigit(const char& ch)
    {
        return ch == '0' || ch == '1';
    }

    bool hexdigit(const char& ch)
    {
        return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
    }

    bool alpha(const char& ch)
    {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    }

    bool additionOperator(const char& ch)
    {
        return ch == '+';
    }

    bool subtractionOperator(const char& ch)
    {
        return ch == '-';
    }

    bool multOperator(const char& ch)
    {
        return ch == '*';
    }

    bool divOperator(const char& ch)
    {
        return ch == '/';
    }

    bool modOperator(const char& ch)
    {
        return ch == '%';
    }

    bool isOperator(const char& ch) {
        return assignment(ch) ||
            modOperator(ch) ||
            divOperator(ch) ||
            multOperator(ch) ||
            subtractionOperator(ch) ||
            additionOperator(ch);
    }

    bool endOfStatement(const char& ch)
    {
        return ch == ';';
    }

    bool assignment(const char& ch)
    {
        return (ch == ':' || ch == '=');
    }

    bool strLitQuote(const char& ch)
    {
        return (ch == '"');
    }

    bool isNewLine(const char& ch)
    {
        if (ch == '\n') {
            return true;
        }

        if (ch == '\r') {
            if (currentText()[currentChIndex + 1] == '\n') {
                return true;
            }
        }

        return false;
    }

    bool openBlock(const char& ch)
    {
        return (ch == '{');
    }

    bool closeBlock(const char& ch)
    {
        return (ch == '}');
    }

    bool openParen(const char& ch)
    {
        return (ch == '(');
    }

    bool closeParen(const char& ch)
    {
        return (ch == ')');
    }

    bool reservedCh(const char& ch)
    {
        return openParen(ch)||closeParen(ch)||openBlock(ch)||
            closeBlock(ch)|| endOfStatement(ch) || isOperator(ch)||
            comment(ch)|| commentStart(ch) || commentEnd(ch);
    }

    bool comment(const char& ch)
    {
        return ch == '#';
    }

    bool commentStart(const char& ch)
    {
        return comment(currentCh(-1)) && ch == '{' ;
    }

    bool commentEnd(const char& ch)
    {
        return ch == '}' && comment(currentCh(1));
    }

    enum State {
        NONE = 0,
        WHITESPACE = 1,
        DIGIT = 2,
        A_Z = 3,
        A_Z_DIGIT = 4,
        DECIMAL_DIGIT = 5,
        HEXADECIMAL_DIGIT,
        BINARY_DIGIT,
        ADD_OPERATOR,
        SUB_OPERATOR,
        MULT_OPERATOR,
        DIV_OPERATOR,
        MOD_OPERATOR,
        IDENTIFIER,
        END_OF_STATEMENT,
        ASSIGNMENT,
        STRING,
        OPEN_PAREN,
        CLOSE_PAREN,
        OPEN_BRACE,
        CLOSE_BRACE,
        OPEN_BRACKET,
        CLOSE_BRACKET,
        COMMENTS,
        COMMENT_OPEN_BRACE,
        COMMENT_CLOSE_BRACE,
        ERROR,
        UNKNOWN = 0xFFFFFFFF
    };

    std::deque<State> stateStack;
    std::string currentLexeme;
    Token currentToken;    
    size_t currentChIndex;
    size_t currentLexemeStart;
    size_t currentLexemeEnd;
    size_t currentLine;    
    std::string currentFilename;
    
    std::vector<std::string> keywords;
    std::string reservedCharacters;

    FileContext* currentCtx;
    std::map<std::string,FileContext> files;

    Lexer() : currentChIndex(0), currentLexemeStart(0), currentLexemeEnd(0), currentLine(0), currentCtx(NULL){
        stateStack.push_back(NONE);
        keywords = { "if", "else", "true", "false", "module", "namespace"};
        reservedCharacters = "()[]{};+-/*";
    }
    ~Lexer() {}

    std::string& currentText() {
        FileContext* fc = currentContext();
        if (NULL == fc) {
            throw std::exception("no file context found!");
        }
        return fc->text;
    }

    const std::string& currentText() const {
        const FileContext* fc = currentContext();
        if (NULL == fc) {
            throw std::exception("no file context found!");
        }
        return fc->text;
    }

    const FileContext* currentContext() const {
        auto it = files.find(currentFilename);
        return &it->second;
    }

    FileContext* currentContext() {
        auto it = files.find(currentFilename);
        return &it->second;
    }

    void pushState(State s)
    {
        stateStack.push_back(s);
    }

    State popState()
    {
        State result = stateStack.back();
        stateStack.pop_back();
        if (stateStack.empty()) {
            pushState(NONE);
        }
        return result;
    }

    State currentState() const {
        return stateStack.back();
    }

    const char& currentCh(int offset = 0) const {
        size_t pos = currentChIndex;
        if ((offset < 0) && (abs(offset) >= currentChIndex)) {
            pos = 0;
        }
        else if ((offset > 0) && (currentChIndex + offset >= currentText().size())) {
            pos = currentText().size() - 1;
        }
        else {
            pos = currentChIndex + offset;
        }
        return currentText()[pos];
    }

    void processWhiteSpace(const char& ch) {

        if (isNewLine(ch)) {
            currentContext()->newLines.push_back(currentChIndex);
            currentLine++;
        }
    }
    //1 based array
    size_t getChIndexForLine(size_t line) {
        return currentContext()->newLines[line-1];
    }


    void processToken(State state)
    {
        currentToken.type = Token::UNKNOWN;
        currentToken.text.assign(currentText().c_str() + currentLexemeStart, currentLexemeEnd - currentLexemeStart);
        std::string s = currentToken.text.str();
        switch (state) {
            case STRING: {
                currentToken.type = Token::STRING_LITERAL;
            }
            break;

            case DIGIT: {
                currentToken.type = Token::INTEGER_LITERAL;
            }
            break;

            case DECIMAL_DIGIT: {
                currentToken.type = Token::DECIMAL_LITERAL;
                popState();
            }
            break;

            case HEXADECIMAL_DIGIT: {
                currentToken.type = Token::HEXADECIMAL_LITERAL;
                popState();
            }
            break;

            case BINARY_DIGIT: {
                currentToken.type = Token::BINARY_LITERAL;
                popState();
            }
            break;

            case ADD_OPERATOR: {
                currentToken.type = Token::ADDITION_OPERATOR;
            }
            break;

            case SUB_OPERATOR: {
                currentToken.type = Token::SUBTRACTION_OPERATOR;
            }
            break;

            case END_OF_STATEMENT: {
                currentToken.type = Token::END_OF_STATEMENT;
            }
            break;

            case OPEN_PAREN: {
                currentToken.type = Token::OPEN_PAREN;
            }
            break;

            case CLOSE_PAREN: {
                currentToken.type = Token::OPEN_PAREN;
            }
            break;

            case OPEN_BRACE: {
                currentToken.type = Token::OPEN_BLOCK;
            }
            break;

            case CLOSE_BRACE: {
                currentToken.type = Token::CLOSE_BLOCK;
            }
            break;

            case ASSIGNMENT: {
                currentToken.type = Token::ASSIGMENT_OPERATOR;
            }
            break;

            case A_Z: case A_Z_DIGIT: {
                currentToken.type = Token::IDENTIFIER;

                if (std::find(keywords.begin(), keywords.end(), currentToken.text.str()) != keywords.end()) {
                    currentToken.type = Token::KEYWORD;
                    if (currentToken.text.str() == "true" || currentToken.text.str() == "false") {
                        currentToken.type = Token::BOOLEAN_LITERAL;
                    }
                }
            }
            break;

            case COMMENT_OPEN_BRACE: {
                currentToken.type = Token::COMMENT_START;
            }
            break;

            case COMMENT_CLOSE_BRACE: {
                currentToken.type = Token::COMMENT_END;
            }
                                   break;

            case COMMENTS: {
                currentToken.type = Token::COMMENT;
            }
            break;
        }
        popState();
        currentLexeme = "";

        currentContext()->tokens.push_back(currentToken);
    }

    void error()
    {
        clearState();
        pushState(ERROR);
    }

    bool processCharacter(const char& ch) {
        bool result = true;
        bool tokenReady = false;
        bool appendToLexeme = false;
        bool reprocessCh = false;

        State s = currentState();
        switch (s) {
            case NONE: {
                if (whitespace(ch)) {
                    pushState(WHITESPACE);
                    processWhiteSpace(ch);
                }
                else if (alpha(ch)) {
                    pushState(A_Z);
                    reprocessCh = true;
                }
                else if (digit(ch)) {
                    pushState(DIGIT);
                    reprocessCh = true;
                }
                else if (additionOperator(ch)) {
                    pushState(ADD_OPERATOR);
                    reprocessCh = true;
                }
                else if (subtractionOperator(ch)) {
                    pushState(SUB_OPERATOR);
                    reprocessCh = true;
                }
                else if (endOfStatement(ch)) {
                    pushState(END_OF_STATEMENT);
                    reprocessCh = true;
                }
                else if (assignment(ch)) {
                    pushState(ASSIGNMENT);
                    reprocessCh = true;
                }
                else if (strLitQuote(ch)) {
                    pushState(STRING);
                }
                else if (openParen(ch)) {
                    pushState(OPEN_PAREN);
                    reprocessCh = true;
                }
                else if (closeParen(ch)) {
                    pushState(CLOSE_PAREN);
                    reprocessCh = true;
                }
                else if (openBlock(ch)) {
                    pushState(OPEN_BRACE);
                    reprocessCh = true;
                }
                else if (closeBlock(ch)) {
                    pushState(CLOSE_BRACE);
                    reprocessCh = true;
                }
                else if (comment(ch)) {
                    pushState(COMMENTS);                    
                }
            }
            break;

            case WHITESPACE: {
                if (!whitespace(ch)) {
                    popState();
                    reprocessCh = true;
                }
                else {
                    processWhiteSpace(ch);
                }
            }
            break;

            case A_Z: {
                if (digit(ch)) {
                    pushState(A_Z_DIGIT);
                    reprocessCh = true;
                }
                else if (ch == '.') {
                    appendToLexeme = true;
                }
                else if (ch == ':') {
                    appendToLexeme = true;
                    tokenReady = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else if (alpha(ch)) {
                    appendToLexeme = true;
                }
                else {
                    error();
                    result = processCharacter(ch);
                }
            }
            break;

            case A_Z_DIGIT: {
                if (ch == '.') {
                    appendToLexeme = true;
                }
                else if (ch == ':') {
                    appendToLexeme = true;
                    tokenReady = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else if (digit(ch) || alpha(ch)) {
                    appendToLexeme = true;
                }
                else {
                    error();
                    result = processCharacter(ch);
                }
            }
            break;

            case DIGIT: {
                if (digit(ch)) {
                    appendToLexeme = true;
                }
                else if (ch == '.') {
                    pushState(DECIMAL_DIGIT);
                    appendToLexeme = true;
                }
                else if ((currentCh(-1) == '0') && (ch == 'x' || ch == 'X')) {
                    pushState(HEXADECIMAL_DIGIT);
                    appendToLexeme = true;
                }
                else if ((currentCh(-1) == '0') && (ch == 'b' || ch == 'B')) {
                    pushState(BINARY_DIGIT);
                    appendToLexeme = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else {
                    error();
                    result = processCharacter(ch);
                }
            }
            break;

            case DECIMAL_DIGIT: {
                if (digit(ch)) {
                    appendToLexeme = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else {
                    error();
                    result = processCharacter(ch);
                }
            }
            break;

            case HEXADECIMAL_DIGIT: {
                if (hexdigit(ch)) {
                    appendToLexeme = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else {
                    error();
                    result = processCharacter(ch);
                }
            }
            break;

            case BINARY_DIGIT: {
                if (binarydigit(ch)) {
                    appendToLexeme = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else {
                    error();
                    result = processCharacter(ch);
                }
            }
            break;

            case STRING: {
                if (!strLitQuote(ch)) {
                    appendToLexeme = true;
                }
                else {
                    tokenReady = true;
                }
            }
            break;

            case ADD_OPERATOR: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case SUB_OPERATOR: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case END_OF_STATEMENT: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case OPEN_PAREN: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case CLOSE_PAREN: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case OPEN_BRACE: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case CLOSE_BRACE: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case ASSIGNMENT: {
                if (!assignment(ch)) {
                    tokenReady = true;
                }
                else {
                    appendToLexeme = true;
                }
            }
            break;

            case COMMENTS: {
                if (commentStart(ch)) {
                    pushState(COMMENT_OPEN_BRACE);        
                    tokenReady = true;
                    appendToLexeme = true;
                }
                else if (commentEnd(ch)) {
                    addToken();

                    pushState(COMMENT_CLOSE_BRACE);                    
                    appendToLexeme = true;
                }
                else if (isNewLine(ch) && currentToken.type!= Token::COMMENT_START ) {
                    tokenReady = true;
                }                
                else {
                    appendToLexeme = true;
                }
            }
            break;
            
            case COMMENT_CLOSE_BRACE: {
                if (comment(ch)) {                    
                    tokenReady = true;
                }
                else {
                    error();
                    result = processCharacter(ch);
                }
            }
            break;
            

            case ERROR: {
                result = false;
            }
            break;
        }

        if (appendToLexeme) {
            if (currentLexemeStart == 0 && currentLexemeEnd == 0) {
                currentLexemeStart = currentChIndex;
                currentLexemeEnd = currentLexemeStart;
            }
            currentLexemeEnd++;
            currentLexeme.append(1, ch);
        }

        if (tokenReady) {
            addToken();
        }

        if (reprocessCh) {
            result = processCharacter(ch);
        }

        return result;
    }

    void addToken() {
        processToken(currentState());
        currentLexemeEnd = 0;
        currentLexemeStart = 0;
    }

    void clearState() {
        stateStack.clear();
        pushState(NONE);
    }

    void clear() {
        currentFilename = "";
        currentChIndex = 0;
        currentLexemeStart = 0;
        currentLexemeEnd = 0;
        currentLine = 1;
        clearState();
    }

    bool lex(const std::string& fileName) {
        std::string text;
        FILE* f = fopen(fileName.c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            size_t fs = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* tmp = new char[fs + 1];
            memset(tmp, 0, fs + 1);
            fread(tmp, 1, fs, f);
            text = tmp;
            delete[] tmp;
            fclose(f);
        }
        else {
            return false;
        }


        return lex(fileName, text);
    }

    bool lex(const std::string& fileName, const std::string& text)
    {
        clear();

        currentFilename = fileName;
        files[currentFilename];

        currentContext()->text = text;

        while (currentChIndex < currentText().size()) {
            const char& ch = currentText()[currentChIndex];
            if (!processCharacter(ch)) {
                return false;
            }
            currentChIndex++;
        }

        return true;
    }


};



#endif //_LEXER_H__

