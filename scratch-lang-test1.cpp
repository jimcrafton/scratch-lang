// new-lang-test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <map>


template<typename chT>
class basic_string_ref {
public:
    typedef chT CharT;
protected:
    const CharT* buffer;
    size_t pos;
    size_t len;
public:
    typedef chT CharT;
    typedef std::basic_string<CharT> StringT;    
    typedef typename StringT::const_iterator  const_iterator;

    enum {
        npos = StringT::npos
    };

    basic_string_ref():buffer(NULL), pos(0), len(0){}
    basic_string_ref(const CharT* b, size_t p, size_t l) :buffer(b), pos(p), len(l) {}

    basic_string_ref& assign(const CharT* b, size_t p, size_t l) {
        buffer = b;
        pos = p;
        len = l;
        return *this;
    }

    const CharT* data() const {
        return buffer;
    }

    const StringT str() const {
        return StringT(&buffer[pos],len);
    }

    size_t size() const {
        return len;
    }
    size_t length() const { return len; }

    bool empty() const { len == 0; }

    size_t offset() const {
        return pos;
    }

    const_iterator begin() const {
        return &buffer[pos];
    }

    const_iterator end() const {
        return &buffer[pos+len];
    }
};


typedef basic_string_ref<char> string_ref;


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
        MESSAGE_SIGNATURE,
    };

    enum Category{
        NONE = 0,
        VARIABLE = 0x0001,
        MESSAGE_PARAM = 0x0010,
        OPERATOR = 0x0002,
        MESSAGE = 0x0100,
    };

    string_ref text;
    Type type;
    Category category;

    Token() :type(UNKNOWN), category(NONE){}

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
            if (currentText[currentChIndex + 1] != '\n') {
                return true;
            }
        }

        return false;
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
        IDENTIFIER ,
        END_OF_STATEMENT,
        ASSIGNMENT,
        STRING,
        ERROR,
        UNKNOWN = 0xFFFFFFFF
    };

    std::deque<State> stateStack;
    std::string currentLexeme;
    Token currentToken;
    std::vector<Token> tokens;
    size_t currentChIndex;
    size_t currentLexemeStart;
    size_t currentLexemeEnd;
    size_t currentLine;
    std::vector<size_t> newLines;
    std::string currentText;
    std::vector<std::string> keywords;
    

    Lexer(): currentChIndex(0), currentLexemeStart(0), currentLexemeEnd(0), currentLine(0){
        stateStack.push_back(NONE);
        keywords = {"if", "else", "true", "false"};
    }
    ~Lexer() {}

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
        else if ((offset > 0) && (currentChIndex+offset >= currentText.size())) {
            pos = currentText.size()-1;
        }
        else {
            pos = currentChIndex + offset;
        }
        return currentText[pos];
    }

    void processWhiteSpace(const char& ch) {
        
        if (isNewLine(ch)) {
            newLines.push_back(currentChIndex);
            currentLine++;
        }
    }


    void processToken(State state)
    {
        currentToken.type = Token::UNKNOWN;
        currentToken.text.assign(currentText.c_str(), currentLexemeStart, currentLexemeEnd- currentLexemeStart);
        std::string s =  currentToken.text.str();
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
        }
        popState();
        currentLexeme = "";

        tokens.push_back(currentToken);
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
                else if (alpha(ch)){
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
                else if (whitespace(ch) || endOfStatement(ch) || isOperator(ch)) {
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
                else if (whitespace(ch) || endOfStatement(ch) || isOperator(ch)) {
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
                else if (whitespace(ch) || endOfStatement(ch) || isOperator(ch)) {
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
                else if (whitespace(ch) || endOfStatement(ch) || isOperator(ch)) {
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
                else if (whitespace(ch) || endOfStatement(ch) || isOperator(ch)) {
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
                else if (whitespace(ch) || endOfStatement(ch) || isOperator(ch)) {
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

            case ASSIGNMENT: {
                if (!assignment(ch)) {
                    tokenReady = true;
                }
                else {
                    appendToLexeme = true;
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
            processToken(s);
            currentLexemeEnd=0;
            currentLexemeStart = 0;
        }

        if (reprocessCh) {
            result = processCharacter(ch);
        }

        return result;
    }

    void clearState() {
        stateStack.clear();
        pushState(NONE);
    }

    void clear() {
        currentText = "";
        currentChIndex = 0;
        currentLexemeStart = 0;
        currentLexemeEnd = 0;
        currentLine = 0;
        clearState();        
        newLines.clear();
        tokens.clear();
    }

    void process(const std::string& text)
    {
        clear();
        currentText = text;

        while (currentChIndex < currentText.size()) {
            const char& ch = currentText[currentChIndex];
            if (!processCharacter(ch)) {
                break;
            }
            currentChIndex++;
        }
    }


};


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
    
    p.process(fileText);


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
        {Token::MESSAGE_SIGNATURE,"MESSAGE_SIGNATURE"}
    };
    for (const Token& t : p.tokens) {
        std::cout << ttmap[t.type] << ": '" << t.text.str() << "'" << std::endl;
    }
    return 0;
}

