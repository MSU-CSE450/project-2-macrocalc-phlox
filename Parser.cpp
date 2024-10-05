#include <vector>

#include "ASTNode.hpp"
#include "lexer.hpp"
class Parser {
 private:
  size_t token_id{0};
  std::vector<emplex::Token> tokens;

 public:
  Parser(std::vector<emplex::Token> tokens) { tokens = tokens; }

  emplex::Token CurrentToken() { return tokens[token_id]; }



  void Parse() { // Some code taken from WordLang.cpp

    while (token_id < tokens.size()) {
    ASTNode cur_node = ParseStatement();
    if (cur_node.GetType()) root.AddChild(cur_node);
  }

  }

  /*
   ASTNode ParseStatement() // Taken from WordLang
   {

        switch (CurrentToken()) {
          using namespace emplex;
          case Lexer.ID_Print: return ParsePrint();
          case Lexer::ID_Type: return ParseDeclare();
          case ';': return ASTNode{0};
          default:
            return ParseExpression();
       }
     }
  */
};