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
#include "lexer.hpp"

bool PRINT_DEBUG = false;

template <typename... Ts>
void Error(size_t line_num, Ts... message) {
  std::cerr << "ERROR (line " << line_num << "): ";
  (std::cerr << ... << message);
  std::cerr << std::endl;
  exit(1);
}


class SymbolTable {
 private:
  struct SymbolInfo {
    std::string name;
    double val;
    size_t declare_line;
    size_t current_id = 0;

    SymbolInfo(std::string name, size_t declare_line)
        : name(name), declare_line(declare_line) {}
  };

  std::vector<SymbolInfo> var_info;
  using scope_t = std::unordered_map<std::string, size_t>;
  std::vector<scope_t> scope_stack{1};

 public:
  static constexpr size_t NO_ID = static_cast<size_t>(-1);

  size_t GetNumVars() const { return var_info.size(); }

  size_t GetVarID(std::string name) const {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      if (it->count(name)) return it->find(name)->second;
    }

    return NO_ID;
  }

  bool HasVar(std::string name) const { return (GetVarID(name) != NO_ID); }

  size_t AddVar(size_t line_num, std::string name) {
    if(PRINT_DEBUG) std::cout << "AddVar line_num: " << line_num << " name: " << name << std::endl;
    auto& scope = scope_stack.back();
    if (scope.count(name)) {
      Error(line_num, "Redeclaration of variable '", name, "'.");
    }
    size_t var_id = var_info.size();
    var_info.emplace_back(name, line_num);
    scope[name] = var_id;
    return var_id;
  }

  double& VarValue(size_t id) {
    if(PRINT_DEBUG) std::cout << "VarValue id: " << id << std::endl;
    assert(id < var_info.size());
    return var_info[id].val;
  }

  void IncScope() { scope_stack.emplace_back(); }

  void DecScope() {
    assert(scope_stack.size() > 1);
    scope_stack.pop_back();
  }
};

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
    if(PRINT_DEBUG) std::cout << "MakeVarNode var_id found in symbol table: " << var_id << "of lexeme: " << token.lexeme << std::endl;
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
    if(PRINT_DEBUG) std::cout << "ParseStatement called CurToken: " << CurToken().lexeme << std::endl;
    switch (CurToken()) {
      using namespace emplex;
      case Lexer::ID_Print:
        return ParsePrint();
        break;
      case Lexer::ID_EOL:
        return ASTNode{};
        break;

      case Lexer::ID_Var: {
        return ParseDeclare();
        break;
      }
      

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
    if(PRINT_DEBUG) std::cout << "ParsePrint called CurToken: " << CurToken().lexeme << std::endl;
    ASTNode print_node(ASTNode::Type::PRINT);
    
    UseToken(emplex::Lexer::ID_Print);
    UseToken(emplex::Lexer::ID_StartCondition);
    
    print_node.AddChild(ParseExpression());
    
    UseToken(emplex::Lexer::ID_EndCondition);
    UseToken(emplex::Lexer::ID_EOL);
    
    return print_node;
  }

  ASTNode ParseDeclare() {
    if(PRINT_DEBUG) std::cout << "ParseDeclare called CurToken: " << CurToken().lexeme << std::endl;
    auto var_token = UseToken(emplex::Lexer::ID_Var);
    auto id_token = UseToken(emplex::Lexer::ID_VariableName);

    symbols.AddVar(var_token,id_token.lexeme);
    
    if(UseTokenIf(';')) return ASTNode{};

    UseToken(emplex::Lexer::ID_Equal, "Expected ';' or '='.");

    auto lhs_node = MakeVarNode(id_token);
    auto rhs_node = ParseExpression();

    UseToken(emplex::Lexer::ID_EOL);
    return ASTNode{ASTNode::ASSIGN, lhs_node, rhs_node};

  }

  ASTNode ParseExpression() {
    if(PRINT_DEBUG) std::cout << "ParseExpression called CurToken: " << CurToken().lexeme << std::endl;
    using namespace emplex;
    ASTNode term_node = ParseTerm();

    auto token = CurToken();
    /*
    switch(token) {
      case Lexer::ID_Equal: return ParseExpressionAssign(term_node);
    }
    */

    if(token == emplex::Lexer::ID_Equal) {
      //
      return ParseExpressionAssign(term_node);
    }

    return term_node;
  }

  ASTNode ParseExpressionAssign(ASTNode left) {
    if(PRINT_DEBUG) std::cout << "ParseExpressionAssign called CurToken: " << CurToken().lexeme << "left: " << left.NodeType() << std::endl;
    using namespace emplex;
    UseToken(Lexer::ID_Equal);

    auto right = ParseExpression();

    if(left.NodeType() == ASTNode::VAR ) {
      return ASTNode{ASTNode::ASSIGN, left, right};
    } else {
      // Error
    }
  }

  ASTNode ParseTerm() {
    if(PRINT_DEBUG) std::cout << "ParseTerm called CurToken: " << CurToken().lexeme << std::endl;
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

      case Lexer::ID_VariableName: {
        return MakeVarNode(token);
      }
      
      default:
        Error(token, "Expected expression. Found ", TokenName(token), ".");
    }

    return ASTNode{};
  }

  double Run(ASTNode& node) {
    switch (node.NodeType()) {


      case ASTNode::EMPTY: {
        if(PRINT_DEBUG) std::cout << "Run: EMPTY case node.GetVal(): " << node.GetVal() << std::endl;
        assert(false);
        break;
      }

      case ASTNode::STATEMENT_BLOCK : {
        if(PRINT_DEBUG) std::cout << "Run: STATEMENT_BLOCK case node.GetVal(): " << node.GetVal() << std::endl;
        for(auto &child : node.GetChildren()){
          
          Run(child);
        }
        return 0.0;
      }
      case ASTNode::PRINT: {
        if(PRINT_DEBUG) std::cout << "Run: PRINT case node.GetVal(): " << node.GetVal() << std::endl;
        for (ASTNode& child : node.GetChildren()) {
          double result = Run(child);
          std::cout << result << std::endl;
        }

        break;
      }

      case ASTNode::VAL : {
        if(PRINT_DEBUG) std::cout << "Run: VAL case node.GetVal(): " << node.GetVal() << std::endl;
        return node.GetVal();
        
      }

      case ASTNode::VAR: {
        if(PRINT_DEBUG) std::cout << "Run: VAR case node.GetVal(): " << node.GetVal() << std::endl;
        assert(node.GetChildren().size() == 0);
        return symbols.VarValue(node.GetVal());
      }

      case ASTNode::ASSIGN: {
        if(PRINT_DEBUG) std::cout << "Run: ASSIGN case node.GetVal(): " << node.GetVal() << std::endl;
        assert(node.GetChildren().size() == 2);
        assert(node.GetChild(0).NodeType() == ASTNode::VAR);
        if(PRINT_DEBUG) std::cout << "  node.GetChild(0).GetVal(); : " << node.GetChild(0).GetVal() << std::endl;
        size_t var_id = node.GetChild(0).GetVal();
        return symbols.VarValue(var_id) = Run(node.GetChild(1));
      }
    }

    return 0.0;
  }

  void Run() { Run(root); }

  void PrintDebug(const ASTNode & node, std::string prefix="") const {
    std::cout << prefix;

    switch (node.NodeType()) {
    case ASTNode::EMPTY:
      std::cout << "EMPTY" << std::endl;
      break;
    case ASTNode::STATEMENT_BLOCK:
      std::cout << "STATEMENT_BLOCK" << std::endl;
      break;
    case ASTNode::ASSIGN:
      std::cout << "ASSIGN" << std::endl;
      break;

    case ASTNode::VAR:
      std::cout << "VARIABLE" << std::endl;
      break;


    case ASTNode::PRINT:
      std::cout << "PRINT" << std::endl;
      break;

    case ASTNode::VAL:
      std::cout << "VAL" << std::endl;
      break;


    }

    for (const auto & child : node.GetChildren()) {
      PrintDebug(child, prefix+"  ");
    }
  }

  void PrintDebug() { PrintDebug(root); }
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


  if(PRINT_DEBUG) {
    std::cout << in_file.rdbuf();
  }

  MacroCalc calc(filename);

  //calc.PrintDebug();

  calc.Run();
}
