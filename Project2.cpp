#include <assert.h>

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Below are some suggestions on how you might want to divide up your project.
// You may delete this and divide it up however you like.
#include "ASTNode.hpp"
#include "SymbolTable.hpp"
#include "lexer.hpp"

template <typename... Ts>
void Error(size_t line_num, Ts... message) {
  std::cerr << "ERROR (line " << line_num << "): ";
  (std::cerr << ... << message);
  std::cerr << std::endl;
  exit(1);
}

class MacroCalc {
 private:
  size_t token_id = 0;
  std::vector<emplex::Token> tokens{};
  ASTNode root{ASTNode::STATEMENT_BLOCK};

  SymbolTable symbols{};

  // === HELPER FUNCTIONS ===

  std::string TokenName(int id) const {
    if (id > 0 && id < 128) {
      return std::string("'") + static_cast<char>(id) + "'";
    }
    return emplex::Lexer::TokenName(id);
  }

  emplex::Token CurToken() const { return tokens[token_id]; }

  emplex::Token UseToken() { return tokens[token_id++]; }

  emplex::Token UseToken(int required_id, std::string err_message = "") {
    if (CurToken() != required_id) {
      if (err_message.size())
        Error(CurToken(), err_message);
      else {
        Error(CurToken(), "Expected token type ", TokenName(required_id),
              ", but found ", TokenName(CurToken()));
      }
    }
    return UseToken();
  }

  bool UseTokenIf(int test_id) {
    if (CurToken() == test_id) {
      token_id++;
      return true;
    }
    return false;
  }

  ASTNode MakeVarNode(const emplex::Token& token) {
    size_t var_id = symbols.GetVarID(token.lexeme);
    assert(var_id < symbols.GetNumVars());
    ASTNode out(ASTNode::VAR);
    out.SetVal(var_id);
    return out;
  }

 public:
  MacroCalc(std::string filename) {  // Looked at WordLang.cpp for this
    std::ifstream file(filename);
    emplex::Lexer lexer;
    tokens = lexer.Tokenize(file);

    Parse();
  }

  void Parse() {
    while (token_id < tokens.size()) {
        ASTNode cur_node = ParseStatement();
        token_id++;
        if (cur_node.NodeType()) root.AddChild(cur_node);
    }
  }

  ASTNode ParseStatement() {
    switch (CurToken()) {
      using namespace emplex;
      case Lexer::ID_Print:
        return ParsePrint();

      /*
      case Lexer::ID_Type:
        return ParseDeclare();
      // case Lexer::ID_IF: return ParseIf();
      // case Lexer::ID_WHILE: return ParseWhile();
      case '{':
        return ParseStatementBlock();
      case ';':
        return ASTNode{};
      */
      default:
        return ParseExpression();
    }
  }

  ASTNode ParsePrint() {
    ASTNode print_node(ASTNode::PRINT);

    UseToken(emplex::Lexer::ID_Print);
    UseToken(245);
    do{
      print_node.AddChild( ParseExpression() );
    }while (UseTokenIf(','));
    UseToken(243);
    UseToken(246);

    return print_node;
    std::cout << "Print" << std::endl;



    return ASTNode{ASTNode::PRINT};

  }

  ASTNode ParseDeclare() {
    std::cout << "Print" << std::endl;



    return ASTNode{ASTNode::PRINT};

  }

  ASTNode ParseExpression() {
    ASTNode term_node = ParseTerm();

    // @CAO - Need to handle operators.

    return term_node;
    std::cout << "HIIIII" << std::endl;
    return ASTNode{0};
  }
  ASTNode ParseTerm() {
    auto token = UseToken();
    switch(token)
    {
      case 247: {
        std::cout << token.lexeme << std::endl;
        break;
      }
      case 244:{
        for(auto ch : token.lexeme)
        {
          if(ch != '"')
          {
            std::cout << ch;
          }
        }
        std::cout << std::endl;
        break;
      }

    }
    return ASTNode{};
  }


};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Format: " << argv[0] << " [filename]" << std::endl;
    exit(1);
  }

  std::string filename = argv[1];

  std::ifstream in_file(filename);  // Load the input file
  if (in_file.fail()) {
    std::cout << "ERROR: Unable to open file '" << filename << "'."
              << std::endl;
    exit(1);
  }

  // TO DO:
  // PARSE input file to create Abstract Syntax Tree (AST).
  // EXECUTE the AST to run your program.

  MacroCalc calc(filename);
}
