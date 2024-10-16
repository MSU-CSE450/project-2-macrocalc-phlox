#include <assert.h>

#include <cmath>
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

  ASTNode MakeVarNode(const emplex::Token& token) {  // Wordlang Stuff
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
      ASTNode curNode = ParseStatement();
      if (curNode.NodeType()) root.AddChild(curNode);
    }
  }

  ASTNode ParseStatement() {
    switch (CurToken()) {
      using namespace emplex;
      case Lexer::ID_Print:
        return ParsePrint();
      case Lexer::ID_EOL:
        return ASTNode{};

      default:
        return ParseExpression();
    }
  }
  /*
  ASTNode ParseStatementBlock() {
  }
  */

  ASTNode ParseIf() {}

  ASTNode ParseWhile() {}
  ASTNode ParsePrint() {
    ASTNode print_node(ASTNode::Type::PRINT);

    UseToken(emplex::Lexer::ID_Print);
    UseToken(emplex::Lexer::ID_StartCondition);
        
    print_node.AddChild(ParseExpression());
  
    UseToken(emplex::Lexer::ID_EndCondition);
    UseToken(emplex::Lexer::ID_EOL);


    return print_node;
  }

  ASTNode ParseDeclare() {}

  ASTNode ParseExpression() {
    ASTNode term_node = ParseTerm();

    return term_node;
  }

  ASTNode ParseTerm() {
    auto token = UseToken();

    switch (token) {
      using namespace emplex;

      case Lexer::ID_Value: {
        ASTNode out_node = ASTNode(ASTNode::Type::VAL);
        double val = std::stod(token.lexeme);

        out_node.SetVal(val);
        return out_node;

        break;
      }
      
      default:
        Error(token, "Expected expression. Found ", TokenName(token), ".");
    }

    return ASTNode{};
  }

  double Run(ASTNode& node) {
    switch (node.NodeType()) {


      case ASTNode::EMPTY: {
        assert(false);
        break;
      }

      case ASTNode::STATEMENT_BLOCK : {
        for(auto &child : node.GetChildren()){
          Run(child);
        }
        return 0.0;
      }
      case ASTNode::PRINT: {
        for (ASTNode& child : node.GetChildren()) {
          double result = Run(child);
          std::cout << result;
        }

        break;
      }

      case ASTNode::VAL : {
        return node.GetVal();
      }
    }

    return 0.0;
  }

  void Run() { Run(root); }
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

  calc.Run();
}
