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

  emplex::Token UseToken() { 
    return tokens[token_id++]; 
  }

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
        break;
      case Lexer::ID_Var:
        return ParseDeclare();
      // case Lexer::ID_I
      // case
      case '{':
        return ParseStatementBlock();
        break;
      case ';': return ASTNode{};

      default:
        return ParseExpression();
    }
  }

  ASTNode ParseStatementBlock() {
    ASTNode out_node{ASTNode::STATEMENT_BLOCK};

    return out_node;
  }

  ASTNode ParseIf() {}

  ASTNode ParseWhile() {
    ASTNode while_node(ASTNode::WHILE);

    return while_node;
  }
  ASTNode ParsePrint() {

    ASTNode print_node(ASTNode::Type::PRINT);
    UseToken(emplex::Lexer::ID_Print);
    UseToken(emplex::Lexer::ID_StartCondition);


    do {
      print_node.AddChild(ParseExpression());
    } while (UseTokenIf(','));

    UseToken(emplex::Lexer::ID_EndCondition);
    UseToken(emplex::Lexer::ID_EOL);
    //std::cout << "Reaching return of ParsePrint" << std::endl;
    return print_node;
  }

  ASTNode ParseDeclare() {
    auto var_token = UseToken(emplex::Lexer::ID_Var);
    auto id_token = UseToken(emplex::Lexer::ID_VariableName);
    symbols.AddVar(id_token.lexeme);

    if (UseTokenIf(';')) return ASTNode{};

    UseToken('=', "Expected ';' or '='.");

    auto lhs_node = MakeVarNode(var_token);
    auto rhs_node = ParseExpression();
    UseToken(';');


    return ASTNode(ASTNode::ASSIGN, lhs_node, rhs_node);
  }

  ASTNode ParseExpression() {

    //std::cout << "ParseExpression" << std::endl;

    auto lhs = ParseTerm();


    std::pair<ASTNode, ASTNode::Type> result = ParseOperationAndRhs();

    return ASTNode(result.second, lhs, result.first);

   }

  std::pair<ASTNode, ASTNode::Type> ParseOperationAndRhs() {


    auto op = ParseOperation();

    if(op == ASTNode::EMPTY) {
      return std::make_pair(ASTNode{}, ASTNode::EMPTY);
    }

    auto rhs = ParseTerm();

    if(rhs.NodeType() == ASTNode::EMPTY) {
      return std::make_pair(ASTNode{}, ASTNode::EMPTY);
    }


    return std::make_pair(rhs, op);
  }

  ASTNode::Type ParseOperation() {

    return ASTNode::Type::EMPTY;

  }

  ASTNode ParseTerm() {
    //std::cout << "In ParseTerm" << std::endl;
    auto token = CurToken();

    switch(token) {
      using namespace emplex;

      case Lexer::ID_VariableName:
        return MakeVarNode(token);
      case '(': {
        UseToken('(');
        ASTNode out_node = ParseExpression();
        UseToken(')');
        return out_node;
      }
      case Lexer::ID_Value: {
        ASTNode out_node = ASTNode(ASTNode::Type::VAL);
        double val = std::stod(UseToken().lexeme);
        out_node.SetVal(val);
        return out_node;

      }

      case Lexer::ID_EndCondition : {
        return ASTNode(ASTNode::Type::EMPTY);
        break;

      }
      case Lexer::ID_EOL : {
        return ASTNode(ASTNode::Type::EMPTY);
        break;
      }
      default:
        Error(token, "Expected expression. Found ", TokenName(token), ".");

    }

    return ASTNode{};

  }

  void PrintNode(ASTNode &node) {

    std::cout << "Node type:" << node.NodeType() << std::endl;

    switch(node.NodeType()) {
      case ASTNode::VAL:
        std::cout << "Printing Val in function: " << std::endl;
        break;
      case ASTNode::LITERAL:
        std::cout << "Printing String literal" << std::endl;
        break;
      default:
        break;
    }
  }

  void Run(ASTNode& node) {
    switch (node.NodeType()) {
      case ASTNode::EMPTY:
        assert(false);
      case ASTNode::STATEMENT_BLOCK: {
        for (ASTNode& child : node.GetChildren()) {
          Run(child);
        }

        break;
      }
      case ASTNode::ASSIGN: {
        assert(node.GetChildren().size() == 2);
        assert(node.GetChild(0).NodeType() == ASTNode::VAR);
        auto var_id = node.GetChild(0).GetVal();
        symbols.SetValue(std::to_string(var_id), node.GetChild(1).GetVal());
        break;
      }

      case ASTNode::PRINT: {
        for (ASTNode& child : node.GetChildren()) {
          Run(child);
          PrintNode(child);
          
        }

        break;

      }

      case ASTNode::VAL: {
        assert(node.GetChildren().size() == 0);
        std::cout << "Value in VAL switch case: " << node.GetVal() << std::endl;
        break;
      }

      default:
        break;
    }
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


  //std::cout << "REACHING RUN" << std::endl;
  calc.Run();
}
