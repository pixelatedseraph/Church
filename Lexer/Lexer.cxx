#include<iostream>
#include<print>
#include<memory>
#include<vector>
#include<string>
#include<optional>
#include<stdexcept>
#include<cctype>
#include<fstream>
#include<sstream>

#define ChurchTokens \
    X(TOK_IDENT)     \
    X(TOK_NUM)       \
    X(TOK_EQ)        \
    X(TOK_SC)        \
    X(TOK_EOF)       \
    X(TOK_LET)       \
    X(TOK_FN)        \

typedef enum TokenKind{
    #define X(Token) Token,
        ChurchTokens
    #undef X
}TokenKind;

class Token{
    TokenKind tokenKind;
    std::optional<std::string> lexeme;

    //Converts Enum states to strings
    std::string stringifyTokenStates(){
        switch (tokenKind){
            #define X(Token) case TokenKind::Token: return #Token;
                ChurchTokens
            #undef X
        }
        return "";
    }

public:
    Token();
    Token(TokenKind tokenKind,std::string lexeme) :  tokenKind(tokenKind), lexeme(std::move(lexeme)) {}
    Token(TokenKind tokenKind) : tokenKind(tokenKind),lexeme(std::nullopt){}
    ~Token() = default;

    TokenKind getTokenKind() const{
        return tokenKind;
    }

    std::optional<std::string> getTokenLexeme() const {
        return lexeme;
    }
    //this should be called somewhere
    void updateSpecializedToken(){
        if(tokenKind == TOK_IDENT){
            if(lexeme.value() == "let")
                tokenKind = TOK_LET;
            else if(lexeme.value() == "fn")
                tokenKind = TOK_FN;
        }
    }

    void printToken(){
        if(tokenKind == TOK_IDENT || tokenKind == TOK_NUM){
            std::print("{}({}) ",stringifyTokenStates(),getTokenLexeme().value());
        }
        else 
            std::print("{} ",stringifyTokenStates());
    }

};

class Lexer{
    int currentIndex = 0;
    int currentLine  = 1;
    int currentColn  = 1;

    std::string& sourceCode;
    std::vector<Token> tokenTable; // Stream of tokens produced by the lexer instance
public:
    //Lexer();
    Lexer(std::string& sourceCode) : sourceCode(sourceCode) {}
    
    void skipWhiteSpace(){
        while(currentIndex < sourceCode.size() && 
            (sourceCode[currentIndex] == ' ')){
            currentIndex++;
            currentColn++;
        }
    }
    
    char currentChar() const {
        return sourceCode[currentIndex];
    } 

    auto lookAhead() -> char{
        return sourceCode[currentIndex + 1];
    }

    void moveAhead() {
        currentIndex++;
        currentColn++;
    }

    auto getNextToken() -> Token{
        skipWhiteSpace();

        //EOF
        if(currentIndex >= sourceCode.size()){
            return Token(TOK_EOF);
        }
        //Identifier
        if(std::isalpha(currentChar())){
            std::string ident;
            while(currentIndex < sourceCode.size() && std::isalnum(currentChar())){
                ident += currentChar();
                moveAhead();
            }
            return Token(TOK_IDENT,ident);
        }
        //Number
        if(std::isdigit(currentChar())){
            std::string number;
            while(currentIndex < sourceCode.size() && std::isdigit(currentChar())){
                number += currentChar();
                moveAhead();
            }
            return Token(TOK_NUM,number);
        }
        //SingleEquals
        if(currentChar() == '='){
            moveAhead();
            return Token(TOK_EQ);
        }
        //SemiColon
        if(currentChar() == ';'){
            moveAhead();
            return Token(TOK_SC);
        }
        //Escape Sequence \n->linux & macos ,windows compat will be added later
        if(currentChar() == '\n'){
            currentLine++;
            currentColn = 1;
            moveAhead();
            return getNextToken();
        }    
        throw std::runtime_error(
            "Stray Token " +
            std::string(1,currentChar()) + " line: "+std::to_string(currentLine) +" at column: "+
            std::to_string(currentColn) );
    }

    void incrementLineCount(){
        currentLine++;
    }

    void lex(){
        while(true){
            Token token = getNextToken();
            token.updateSpecializedToken();
            if(token.getTokenKind() == TOK_EOF)
                break;
            token.printToken();

        }
    }
};

int main(int argc,char** argv){

    if(argc != 2){
        std::cerr<<"Fatal error: No input file\n";
        std::cerr<<"compilation terminated.\n";
        return 1;
    }

    std::ifstream churchSourceFile (argv[1]); // the file with .ch source

    if(!churchSourceFile){
        std::cerr<<"Fatal error: "<<argv[1]<<" : No such file or directory\n";
        std::cerr<<"compilation terminated.\n";
        return 2;
    }
    std::stringstream churchStringBuffer;
    churchStringBuffer << churchSourceFile.rdbuf();

    std::string churchSourceCode = churchStringBuffer.str();

    Lexer lexer(churchSourceCode);
    lexer.lex();

    return 0;
}

