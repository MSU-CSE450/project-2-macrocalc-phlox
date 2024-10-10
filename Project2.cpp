#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Below are some suggestions on how you might want to divide up your project.
// You may delete this and divide it up however you like.
#include "ASTNode.hpp"
#include "lexer.hpp"
#include "SymbolTable.hpp"

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
    std::vector<emplex::Token> tokens;
    std::vector<emplex::Token> tokens{};
    size_t token_id{0};
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

    emplex::Token UseToken(int required_id, std::string err_message="") {
      if (CurToken() != required_id) {
        if (err_message.size()) Error(CurToken(), err_message);
        else {
          Error(CurToken(),
            "Expected token type ", TokenName(required_id),
            ", but found ", TokenName(CurToken())
          );
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

    ASTNode MakeVarNode(const emplex::Token & token) {
      size_t var_id = symbols.GetVarID(token.lexeme);
      assert(var_id < symbols.GetNumVars());
      ASTNode out(ASTNode::VAR);
      out.SetVal(var_id);
      return out;
    }




  public:
    MacroCalc(std::string filename) { // Looked at WordLang.cpp for this
      std::ifstream file(filename);
      emplex::Lexer lexer;
      tokens = lexer.Tokenize(file);

      Parse();

    }

    void Parse() {
      while(token_id < tokens.size()) {
        //std::cout << tokens[token_id] << std::endl;
        token_id++;
      }

    }

};

int main(int argc, char * argv[])
{
  if (argc != 2) {
    std::cout << "Format: " << argv[0] << " [filename]" << std::endl;
    exit(1);
  }

  std::string filename = argv[1];
  
  std::ifstream in_file(filename);              // Load the input file
  if (in_file.fail()) {
    std::cout << "ERROR: Unable to open file '" << filename << "'." << std::endl;
    exit(1);
  }



  // TO DO:  
  // PARSE input file to create Abstract Syntax Tree (AST).
  // EXECUTE the AST to run your program.

  MacroCalc calc(filename);






  
}
