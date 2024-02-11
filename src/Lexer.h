#ifndef _LEXER_H__
#define _LEXER_H__

#pragma once

#include "string_ref.h"
#include "Token.h"
#include <cstdarg>



class FileContext {
public:
    std::string fileName;    
    std::vector<Token> tokens;
    std::vector<size_t> newLines;
    std::string text;
    mutable std::vector<Token>::const_iterator curTokIt;

    void clear() {
        tokens.clear();
        newLines.clear();
        text = "";
    }

    void beginTokens() const  {
        curTokIt = tokens.begin();
    }

    const Token& getCurrentToken() const {
        return *curTokIt;
    }

    const Token& getCurrentToken(int afterPos) const {
        auto it = curTokIt+afterPos;
        return *it;
    }

    const Token& lastToken() const {
        auto it = curTokIt + (tokens.size()-1);
        return *it;
    }

    bool prevToken() const {        

        if (curTokIt == tokens.begin()) {
            return false;
        }

        --curTokIt;

        return true;
    }

    bool nextToken() const {
        ++curTokIt;

        if (curTokIt == tokens.end()) {
            return false;
        }
        
        return true;
    }

    std::string getLine(size_t pos) const {
        std::string result;

        size_t startPos = pos;
        const char* P = text.c_str();
        P += startPos;
        while (startPos > 0) {
            if (*P == '\n') {
                startPos++;
                P++;
                break;
            }
            P--;
            startPos--;
        }

        size_t endPos = startPos;
        while (*P != '\n' && *P != '\n') {
            endPos++;
            P++;
        }

        size_t len = (endPos - startPos) < (text.length() - startPos) ?
            (endPos - startPos) : (text.length() - startPos);

        result = text.substr(startPos, len);

        return result;
    }
    
};


class Lexer {
public:

    class Error {
    public:
        size_t line = 0;
        size_t col = 0;
        size_t offset = 0;
        std::string message;

        void clear() {
            line = 0;
            col = 0;
            offset = 0;
            message = "";
        }

        void output(const Lexer& l) const {
            std::string errorText = l.currentContext()->getLine(offset);
            errorText += "\n\n";
            std::string spacer;
            if (col > 1) {
                spacer.append(col - 1, '-');
            }
            errorText += spacer + "^" + "\n";
            std::cout << errorText << "Error: " << message << " at line : " << line << ", " << col << std::endl;
        }
    };

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
        return (colon(ch) && equalsSign(currentCh(1)));
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
            if (currentCh(1) == '\n') {
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
            comment(ch)|| commentStart(ch) || commentEnd(ch) || comma(ch);
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

    bool comma(const char& ch) {
        return ch == ',';
    }

    bool colon(const char& ch) {
        return ch == ':';
    }

    bool equalsSign(const char& ch)
    {
        return ch == '=';
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
        VERSION_DIGIT,
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
        COMMA,
        COLON,
        EQUALS_SIGN,
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
    size_t currentCol;
    std::string currentFilename;
    
    Error lexError;

    std::vector<std::string> keywords;
    std::string reservedCharacters;

    FileContext* currentCtx;
    std::map<std::string,FileContext> files;

    Lexer() : currentChIndex(0), currentLexemeStart(0), currentLexemeEnd(0), currentLine(0), currentCol(0),currentCtx(NULL){
        stateStack.push_back(NONE);
        initKeywords();
        initReservedCharacters();
    }
    ~Lexer() {}

    void initReservedCharacters() {
        reservedCharacters = "()[]{};+-/*:";
    }

    void initKeywords() {
        keywords = { "if", 
            "else", 
            "true", 
            "false",             
            "main",
            "module",
            "program",
            "lib",
            "import",
            "namespace",
            "class",
            "inherits",
            "implements",
            "private",
            "public",
            "static",
            "return"
        };
    }


    const Error& getError() const { return lexError; }

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

    size_t getPosFromOffset(int offset) const {
        size_t result = currentChIndex;
        if ((offset < 0) && (abs(offset) >= currentChIndex)) {
            result = 0;
        }
        else if ((offset > 0) && (currentChIndex + offset >= currentText().size())) {
            result = currentText().size() - 1;
        }
        else {
            result = currentChIndex + offset;
        }
        return result;
    }

    const char& currentCh(int offset = 0) const {
        size_t pos = getPosFromOffset(offset);
        return currentText()[pos];
    }

    std::string getCurrentText(int startOffset, int endOffset) {
        std::string result;

        size_t startPos = getPosFromOffset(startOffset);
        size_t endPos = getPosFromOffset(endOffset);

        if (startPos >= endPos) {
            printf("Error, %zu >= %zu\n", startPos, endPos);
            return result;
        }

        result = currentText().substr(startPos, endPos - startPos);

        return result;
    }

    std::string getCurrentLine() const {

        return currentContext()->getLine(getPosFromOffset(0));
    }

    
    void processWhiteSpace(const char& ch) {

        if (isNewLine(ch)) {
            if (!(ch == '\n' && currentText()[currentChIndex - 1] == '\r')) {
                currentContext()->newLines.push_back(currentChIndex);
                currentLine++;
                currentCol = 0;
            }
        }
    }
    //1 based array
    size_t getChIndexForLine(size_t line) {
        return currentContext()->newLines[line-1];
    }


    void processToken(State state)
    {
        currentToken.type = Token::UNKNOWN;
        currentToken.lineNumber = currentLine;
        currentToken.colNumber = currentCol;
        currentToken.text.assign(currentText().c_str() + currentLexemeStart, currentLexemeEnd - currentLexemeStart);
        currentToken.offset = currentLexemeStart;
        currentToken.filename = currentFilename;

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
                popState();
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
                if (state == A_Z_DIGIT) {
                    popState();
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

            case COMMA: {
                currentToken.type = Token::COMMA;
                popState();
            }
            break;

            case COLON: {
                currentToken.type = Token::COLON;
                clearState();
            }
            break;

            case VERSION_DIGIT: {
                currentToken.type = Token::VERSION_LITERAL;
                clearState();
            }
            break;
        }
        popState();
        currentLexeme = "";        
        currentContext()->tokens.push_back(currentToken);
    }

    void error_va(const char& ch, const std::string& errString, va_list arg)
    {
        char errbuf[1024];
        vsnprintf(errbuf, sizeof(errbuf) - 1, errString.c_str(), arg);

        clearState();
        lexError.clear();
        
        lexError.line = currentLine;
        lexError.col = currentCol - 1;
        lexError.offset = currentChIndex;
        lexError.message = errbuf;
    }

    void error( const char& ch, std::string errString ...)
    {
        va_list args;
        va_start(args, errString);
        error_va(ch, errString, args);
        va_end(args);
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
                /*else if (assignment(ch)) {
                    
                    reprocessCh = true;
                }
                */
                
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
                else if (equalsSign(ch)) {
                    pushState(EQUALS_SIGN);
                    reprocessCh = true;
                }
                else if (colon(ch)) {
                    pushState(COLON);
                    reprocessCh = true;
                }
                else if (comma(ch)) {
                    pushState(COMMA);
                    reprocessCh = true;
                }
                else {
                    error(ch, "Unknown lex state for character: %c", ch);
                    return false;
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
                if (alpha(ch)) {
                    appendToLexeme = true;
                }
                else if (digit(ch)) {
                    pushState(A_Z_DIGIT);
                    reprocessCh = true;
                }
                else if (ch == '.') {
                    appendToLexeme = true;
                }
                else if (colon(ch)) {                    
                    appendToLexeme = false;
                    tokenReady = true;
                    reprocessCh = true;
                }
                else if (comma(ch)) {                    
                    appendToLexeme = false;
                    tokenReady = true;
                    reprocessCh = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }                
                else {
                    error(ch, "Invalid alphabet char, not in A-z or a-z" );
                    return false;
                }
            }
            break;

            case A_Z_DIGIT: {
                if (ch == '.') {
                    appendToLexeme = true;
                }
                else if (colon(ch)) {
                    pushState(COLON);
                    appendToLexeme = false;
                    tokenReady = true;
                    reprocessCh = true;
                }
                else if (comma(ch)) {                    
                    appendToLexeme = false;
                    tokenReady = true;
                    reprocessCh = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else if (digit(ch) || alpha(ch)) {
                    appendToLexeme = true;
                }
                else {
                    error(ch, "Invalid alpha num char, not in A-z or a-z or 0-9");
                    return false;
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
                    error(ch, "Invalid digits char, not in 0 - 9 or starting with 0X|x, 0B|b");
                    return false;
                }
            }
            break;

            case VERSION_DIGIT: {
                if (digit(ch)) {
                    appendToLexeme = true;
                }
                else if (ch == '.') {
                    appendToLexeme = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else {
                    error(ch, "Invalid version char, not in 0 - 9 or '.'");
                    return false;
                }
            }
            break;

            case DECIMAL_DIGIT: {
                if (digit(ch)) {
                    appendToLexeme = true;
                }
                else if (ch == '.') {
                    pushState(VERSION_DIGIT);
                    appendToLexeme = true;
                }
                else if (whitespace(ch) || reservedCh(ch)) {
                    tokenReady = true;
                    reprocessCh = !whitespace(ch);
                }
                else {
                    error(ch, "Invalid decimal char, not in 0 - 9 or '.'");
                    return false;
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
                    error(ch, "Invalid hexadecimal char, not in 0 - 9 or A-F or starting with 0X|x");
                    return false;
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
                    error(ch, "Invalid binary char, not in 0 - 1 or starting with 0B|b");
                    return false;
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

            case COMMA: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case CLOSE_BRACE: {
                appendToLexeme = true;
                tokenReady = true;
            }
            break;

            case COLON: {
                if (equalsSign(ch)) {
                    pushState(ASSIGNMENT);
                    reprocessCh = true;
                    appendToLexeme = true;
                    tokenReady = false;
                }
                else if (colon(ch) ) {
                    appendToLexeme = true;
                }
                else if (whitespace(ch)) {
                    tokenReady = true;
                }
                else {
                    error(ch, "Expected '=' char");
                    return false;
                }
            }
            break;
            

            case ASSIGNMENT: {
                if (colon(ch) ) {
                    appendToLexeme = true;
                }
                else if (equalsSign(ch) && colon(currentCh(-1)) ) {
                    appendToLexeme = true;
                    tokenReady = true;
                }
                else if (colon(currentCh(-1)) && !equalsSign(ch)) {
                    error(ch, "Expected '=' char, looks like assignment was intended?");
                    return false;
                }
                else {
                    error(ch, "Expected ':=' sequence, looks like assignment was intended?");
                    return false;
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
                    error(ch, "Expected closing brace '}'");
                    return false;
                }
            }
            break;
            

            default: {
                //no idea, assume error
                error(ch, "unhandled character '%c'", ch);
                return false;
            }
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
        currentCol = 0;
        clearState();
        lexError.clear();
    }

    void incrChIdx() {
        ++currentChIndex;
        ++currentCol;
    }

    bool hasMoreText() const {
        return currentChIndex < currentText().size();
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

        while (hasMoreText()) {
            const char& ch = currentText()[currentChIndex];
            if (!processCharacter(ch)) {
                return false;
            }
            incrChIdx();
        }

        currentContext()->beginTokens();

        return true;
    }


};



#endif //_LEXER_H__

