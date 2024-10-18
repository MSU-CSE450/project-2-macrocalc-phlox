#include <assert.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <iomanip>

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
    if (PRINT_DEBUG)
      std::cout << "AddVar line_num: " << line_num << " name: " << name
                << std::endl;
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
    if (PRINT_DEBUG) std::cout << "VarValue id: " << id << std::endl;
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
    if (CurToken().id != required_id) {
      if (err_message.size())
        Error(CurToken().line_id, err_message);
      else {
        Error(CurToken().line_id, "Expected token type ", TokenName(required_id),
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
    if(var_id == SymbolTable::NO_ID) Error(token.line_id, "Undeclared Variable: ", token.lexeme);
    if (PRINT_DEBUG)
      std::cout << "MakeVarNode var_id found in symbol table: " << var_id
                << "of lexeme: " << token.lexeme << std::endl;
    assert(var_id < symbols.GetNumVars());
    ASTNode out(ASTNode::VAR);
    out.SetLineNum(token.line_id);
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
    if (PRINT_DEBUG)
      std::cout << "ParseStatement called CurToken: " << CurToken().lexeme
                << std::endl;
    switch (CurToken()) {
      using namespace emplex;
      case Lexer::ID_Print:
        return ParsePrint();
        break;
      case Lexer::ID_EOL:
        // std::cout << "BING" << std::endl;
        UseToken(Lexer::ID_EOL);
        return ASTNode{};
        break;

      case Lexer::ID_Var: {
        return ParseDeclare();
        break;
      }

      case Lexer::ID_StartScope: {
        return ParseStatementBlock();
      }

      case Lexer::ID_If: {
        return ParseIf();
      }
      case Lexer::ID_While: {
        return ParseWhile();
      }

      default:
        return ParseExpression();
    }
  }

  ASTNode ParseStatementBlock() {
    ASTNode out_node{ASTNode::STATEMENT_BLOCK};
    UseToken(emplex::Lexer::ID_StartScope);
    symbols.IncScope();
    while (CurToken() != emplex::Lexer::ID_Endscope) {
      out_node.AddChild(ParseStatement());
    }
    symbols.DecScope();
    UseToken(emplex::Lexer::ID_Endscope);
    return out_node;
  }

  ASTNode ParseIf() {
    UseToken(emplex::Lexer::ID_If);
    UseToken(emplex::Lexer::ID_StartCondition, "Expected (");

    auto left = ParseExpression();
    UseToken(emplex::Lexer::ID_EndCondition, "Expected )");

    auto right = ParseStatement();
    ASTNode out_node(ASTNode::IF, left, right);

    if (CurToken() == emplex::Lexer::ID_Else) {
      UseToken(emplex::Lexer::ID_Else);

      ASTNode else_statement = ParseStatement();
      out_node.AddChild(else_statement);
    }

    return out_node;
  }

  ASTNode ParseWhile() {
    UseToken(emplex::Lexer::ID_While);
    UseToken(emplex::Lexer::ID_StartCondition, "Expected (");

    auto left = ParseExpression();
    UseToken(emplex::Lexer::ID_EndCondition, "Expected )");
    auto right = ParseStatement();

    ASTNode out_node(ASTNode::WHILE, left, right);

    return out_node;
  }
  ASTNode ParsePrint() {
    if (PRINT_DEBUG)
      std::cout << "ParsePrint called CurToken: " << CurToken().lexeme
                << std::endl;
    ASTNode print_node(ASTNode::Type::PRINT);

    UseToken(emplex::Lexer::ID_Print);
    UseToken(emplex::Lexer::ID_StartCondition);
    std::string literal =
        CurToken().lexeme.substr(1, CurToken().lexeme.length() - 2);
    if (UseTokenIf(emplex::Lexer::ID_LitString)) {
      ASTNode print_node_literal{ASTNode::Type::PRINT_LITERAL};

      print_node_literal.SetLiteral(literal);

      UseToken(emplex::Lexer::ID_EndCondition);
      UseToken(emplex::Lexer::ID_EOL);

      return print_node_literal;
    }

    print_node.AddChild(ParseExpression());

    UseToken(emplex::Lexer::ID_EndCondition);
    UseToken(emplex::Lexer::ID_EOL);

    return print_node;
  }

  ASTNode ParseDeclare() {
    if (PRINT_DEBUG)
      std::cout << "ParseDeclare called CurToken: " << CurToken().lexeme
                << std::endl;
    auto var_token = UseToken(emplex::Lexer::ID_Var);
    auto id_token = UseToken(emplex::Lexer::ID_VariableName);
    ASTNode node(ASTNode::VAR, var_token.line_id);
    symbols.AddVar(var_token.line_id, id_token.lexeme);

    if (UseTokenIf(';')) return ASTNode{};

    UseToken(emplex::Lexer::ID_Equal, "Expected ';' or '='.");

    auto lhs_node = MakeVarNode(id_token);
    auto rhs_node = ParseExpression();

    UseToken(emplex::Lexer::ID_EOL);
    return ASTNode{ASTNode::ASSIGN, lhs_node, rhs_node};
  }

  ASTNode ParseExpression() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpression called CurToken: " << CurToken().lexeme
                << std::endl;
    return ParseExpressionAssign();
  }

  // Right assoc. -> Recursively parse with same function
  // Pretty much taken direct from Wordlang
  ASTNode ParseExpressionAssign() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionAssign called CurToken: "
                << CurToken().lexeme << std::endl;

    auto left = ParseExpressionOr();

    if (UseTokenIf(emplex::Lexer::ID_Equal)) {
      if (left.NodeType() != ASTNode::VAR) {
        Error(CurToken().line_id, "The left side of an assignment must be a var");
      }

      ASTNode right = ParseExpressionAssign();
      return ASTNode(ASTNode::ASSIGN, left, right);
    }

    return left;
  }

  /*
   ASTNode ParseExpression() {
     if (PRINT_DEBUG)
       std::cout << "ParseExpression called CurToken: " << CurToken().lexeme
                 << std::endl;
     using namespace emplex;
     ASTNode term_node = ParseTerm();

     auto token = CurToken();
     /*
     switch(token) {
       case Lexer::ID_Equal: return ParseExpressionAssign(term_node);
     }


     if (token == emplex::Lexer::ID_Equal) {
       //
       return ParseExpressionAssign(term_node);
     } else if (token == emplex::Lexer::ID_LitString) {
       ASTNode out_node(ASTNode::Type::LITERAL);

       out_node.SetLiteral(token.lexeme);
       return out_node;
     }

     return term_node;
   }
   */
  /*
  ASTNode ParseExpressionAssign(ASTNode left) {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionAssign called CurToken: "
                << CurToken().lexeme << "left: " << left.NodeType()
                << std::endl;
    using namespace emplex;
    UseToken(Lexer::ID_Equal);

    auto right = ParseExpression();

    if (left.NodeType() == ASTNode::VAR) {
      return ASTNode{ASTNode::ASSIGN, left, right};
    } else {
      // Error
    }
  }
  */

  // Left Assoc. -> Parse with loop (left to right)
  // Inspired from WordLang ParseExpressionAddSub
  ASTNode ParseExpressionOr() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionOr called CurToken: " << CurToken().lexeme
                << std::endl;
    auto left = ParseExpressionAnd();

    while (CurToken() == emplex::Lexer::ID_Or) {
      int token = UseToken();
      ASTNode right = ParseExpressionAnd();
      left = ASTNode{ASTNode::OR, left, right};
      left.SetVal(token);
    }

    return left;
  }
  // Left Assoc. -> Parse with loop (left to right)
  // Inspired from WordLang ParseExpressionAddSub
  ASTNode ParseExpressionAnd() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionAnd called CurToken: " << CurToken().lexeme
                << std::endl;
    auto left = ParseExpressionEquality();

    while (CurToken() == emplex::Lexer::ID_And) {
      int token = UseToken();
      ASTNode right = ParseExpressionEquality();
      left = ASTNode{ASTNode::AND, left, right};
      left.SetVal(token);
    }

    return left;
  }
  // Non Assoc. -> single calls for lhs, rhs (no chaining allowed)
  ASTNode ParseExpressionEquality() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionEquality called CurToken: "
                << CurToken().lexeme << std::endl;
    auto left = ParseExpressionCompare();

    while (CurToken() == emplex::Lexer::ID_Equality ||
           CurToken() == emplex::Lexer::ID_NotEqual) {
      int token = UseToken();
      auto right = ParseExpressionCompare();

      if (token == emplex::Lexer::ID_Equality) {
        left = ASTNode(ASTNode::EQUALITY, left, right);

      } else {
        left = ASTNode(ASTNode::NON_EQUALITY, left, right);
      }
    }

    return left;
  }
  // Non Assoc. -> single calls for lhs, rhs (no chaining allowed)
  ASTNode ParseExpressionCompare() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionCompare called CurToken: "
                << CurToken().lexeme << std::endl;
    auto left = ParseExpressionAddSub();

    while (CurToken() == emplex::Lexer::ID_Less ||
           CurToken() == emplex::Lexer::ID_Greater ||
           CurToken() == emplex::Lexer::ID_GreaterEqual ||
           CurToken() == emplex::Lexer::ID_LessEqual) {
      int token = UseToken();
      auto right = ParseExpressionAddSub();
      if (CurToken() == emplex::Lexer::ID_Less ||
          CurToken() == emplex::Lexer::ID_Greater ||
          CurToken() == emplex::Lexer::ID_GreaterEqual ||
          CurToken() == emplex::Lexer::ID_LessEqual) {
          Error(CurToken().line_id, "Comparisons should be non-associative");
        }
      switch (token) {
        case emplex::Lexer::ID_Less: {
          left = ASTNode(ASTNode::LESS, left, right);
          break;
        }

        case emplex::Lexer::ID_Greater: {
          left = ASTNode(ASTNode::GREATER, left, right);
          break;
        }
        case emplex::Lexer::ID_GreaterEqual: {
          left = ASTNode(ASTNode::GREATER_EQUAL, left, right);
          break;
        }
        case emplex::Lexer::ID_LessEqual: {
          left = ASTNode(ASTNode::LESS_EQUAL, left, right);
          break;
        }
      }
    }

    return left;
  }
  // Left Assoc. -> Parse with loop (left to right)
  // Inspired from WordLang ParseExpressionAddSub
  ASTNode ParseExpressionAddSub() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionAddSub called CurToken: "
                << CurToken().lexeme << std::endl;
    auto left = ParseExpressionModDivMulMod();

    while (CurToken() == emplex::Lexer::ID_Plus ||
           CurToken() == emplex::Lexer::ID_Minus) {
      int token = UseToken();
      auto right = ParseExpressionModDivMulMod();

      if (token == emplex::Lexer::ID_Plus) {
        left = ASTNode{ASTNode::ADD, left, right};
      } else {
        left = ASTNode{ASTNode::SUBTRACT, left, right};
      }

      left.SetVal(token);
    }

    return left;
  }

  // Left Assoc. -> Parse with loop (left to right)
  // Inspired from WordLang ParseExpressionAddSub
  ASTNode ParseExpressionModDivMulMod() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionDivMulMod called CurToken: "
                << CurToken().lexeme << std::endl;
    auto left = ParseExpressionExponentiate();

    while (CurToken() == emplex::Lexer::ID_Multiply ||
           CurToken() == emplex::Lexer::ID_Divide ||
           CurToken() == emplex::Lexer::ID_Modulus) {
      int token = UseToken();
      auto right = ParseExpressionExponentiate();

      if (token == emplex::Lexer::ID_Multiply) {
        left = ASTNode{ASTNode::MULT, left, right};
      } else if (token == emplex::Lexer::ID_Divide) {
        left = ASTNode{ASTNode::DIV, left, right};
      } else {
        left = ASTNode{ASTNode::MOD, left, right};
      }
      left.SetVal(token);
    }

    return left;
  }

  // Right assoc. -> Recursively parse with same function
  // Mirroed the Assign fuinction
  ASTNode ParseExpressionExponentiate() {
    if (PRINT_DEBUG)
      std::cout << "ParseExpressionExponentiate called CurToken: "
                << CurToken().lexeme << std::endl;

    auto left = ParseTerm();

    if (UseTokenIf(emplex::Lexer::ID_Exponent)) {
      ASTNode right = ParseExpressionExponentiate();
      return ASTNode(ASTNode::EXPONENT, left, right);
    }

    return left;
  }

  ASTNode ParseTerm() {
    if (PRINT_DEBUG)
      std::cout << "ParseTerm called CurToken: " << CurToken().lexeme
                << std::endl;
    auto token = UseToken();

    switch (token) {
      using namespace emplex;

      case Lexer::ID_Minus: {
        ASTNode right = ParseTerm();
        return ASTNode(ASTNode::Type::NEGATE, right);
      }

      case Lexer::ID_Negate: {
        ASTNode right = ParseTerm();
        return ASTNode(ASTNode::Type::LOGICAL_NOT, right);
      }

      case Lexer::ID_Value: {
        ASTNode out_node = ASTNode(ASTNode::Type::VAL);
        if (PRINT_DEBUG)
          std::cout << "ParseTerm case ID_VALUE token.lexeme: " << token.lexeme
                    << std::endl;
        double val = std::stod(token.lexeme);

        out_node.SetVal(val);
        return out_node;

        break;
      }

      case Lexer::ID_VariableName: {
        return MakeVarNode(token);
      }

      case Lexer::ID_StartCondition: {
        ASTNode out_node = ParseExpression();
        UseToken(Lexer::ID_EndCondition);
        return out_node;
      }

      default:
        Error(token.line_id, "Expected expression. Found ", TokenName(token), ".");
    }

    return ASTNode{};
  }

  std::string FillInLiteralVariables(std::string in) {
    bool isScope = false;
    std::string output = "";
    std::string currvar = "";
    for (auto ch : in) {
      if (ch != '"') {
        if (ch == '{') {
          isScope = true;
        } else if (ch == '}') {
          if (!symbols.HasVar(currvar)) {
            Error(CurToken().line_id, "Variable does not exist");
          }

          std::ostringstream out;
          out << std::fixed << std::setprecision(5) << symbols.VarValue(symbols.GetVarID(currvar));

          std::string intermediate = out.str();
          int i = intermediate.size();
          
          //int i = std::to_string(symbols.VarValue(symbols.GetVarID(currvar)))
                      //.size();
          //std::string intermediate =
              //std::to_string(symbols.VarValue(symbols.GetVarID(currvar)));
          std::string last_dig = "";
          while (intermediate.back() == '0') {
            if (intermediate.back() == '0') {
              intermediate.pop_back();
            }
          }
          last_dig = intermediate.back();
          if (last_dig == ".") {
            intermediate.pop_back();
          }
          output += intermediate;

          currvar = "";
          isScope = false;
        } else if (isScope == false) {
          output += ch;
        } else {
          currvar += ch;
        }
      }
    }
    return output;
  }

  double Run(ASTNode& node) {
    switch (node.NodeType()) {
      case ASTNode::EMPTY: {
        if (PRINT_DEBUG)
          std::cout << "Run: EMPTY case node.GetVal(): " << node.GetVal()
                    << std::endl;
        assert(false);
        break;
      }

      case ASTNode::STATEMENT_BLOCK: {
        if (PRINT_DEBUG)
          std::cout << "Run: STATEMENT_BLOCK case node.GetVal(): "
                    << node.GetVal() << std::endl;
        for (auto& child : node.GetChildren()) {
          Run(child);
        }
        return 0.0;
      }
      case ASTNode::PRINT: {
        if (PRINT_DEBUG)
          std::cout << "Run: PRINT case node.GetVal(): " << node.GetVal()
                    << std::endl;
        for (ASTNode& child : node.GetChildren()) {
          if (child.NodeType() == ASTNode::Type::LITERAL) {
            std::cout << node.GetLiteral();
          } else {
            double result = Run(child);

            std::cout << result << std::endl;
          }
        }

        break;
      }

      case ASTNode::VAL: {
        if (PRINT_DEBUG)
          std::cout << "Run: VAL case node.GetVal(): " << node.GetVal()
                    << std::endl;
        return node.GetVal();
      }

      case ASTNode::VAR: {
        if (PRINT_DEBUG)
          std::cout << "Run: VAR case node.GetVal(): " << node.GetVal()
                    << std::endl;

        assert(node.GetChildren().size() == 0);
        return symbols.VarValue(node.GetVal());
      }

      case ASTNode::ASSIGN: {
        if (PRINT_DEBUG)
          std::cout << "Run: ASSIGN case node.GetVal(): " << node.GetVal()
                    << std::endl;
        assert(node.GetChildren().size() == 2);
        assert(node.GetChild(0).NodeType() == ASTNode::VAR);
        if (PRINT_DEBUG)
          std::cout << "  node.GetChild(0).GetVal(); : "
                    << node.GetChild(0).GetVal() << std::endl;
        size_t var_id = node.GetChild(0).GetVal();
        return symbols.VarValue(var_id) = Run(node.GetChild(1));
      }

      case ASTNode::PRINT_LITERAL:
        if (PRINT_DEBUG) std::cout << "Run: PRINT_LITERAL case" << std::endl;
        // std::cout << node.GetLiteral() << std::endl;
        std::cout << FillInLiteralVariables(node.GetLiteral()) << std::endl;

        break;
      case ASTNode::ADD: {
        if (PRINT_DEBUG) std::cout << "Run: ADD case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        return left + right;

        break;
      }

      case ASTNode::EXPONENT: {
        if (PRINT_DEBUG) std::cout << "Run: EXPONENT case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        return pow(left, right);

        break;
      }

      case ASTNode::MULT: {
        if (PRINT_DEBUG) std::cout << "Run: MULT case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        return left * right;
        break;
      }

      case ASTNode::DIV: {
        if (PRINT_DEBUG) std::cout << "Run: DIV case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));
        if(right == 0) Error(node.GetLineNum(), "Division by 0");

        return left / right;
        break;
      }

      case ASTNode::MOD: {
        if (PRINT_DEBUG) std::cout << "Run: MOD case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));
        if(right == 0) Error(node.GetLineNum(), "Mod by 0");
        return fmod(left, right);
        break;
      }

      case ASTNode::NEGATE: {
        if (PRINT_DEBUG) std::cout << "Run: NEGATE case" << std::endl;

        assert(node.GetChildren().size() == 1);

        double left = Run(node.GetChild(0));

        return left * -1.0;
        break;
      }

      case ASTNode::SUBTRACT: {
        if (PRINT_DEBUG) std::cout << "Run: SUBTRACT case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        return left - right;
        break;
      }

      case ASTNode::LOGICAL_NOT: {
        if (PRINT_DEBUG) std::cout << "Run: LOGICAL_NOT case" << std::endl;

        assert(node.GetChildren().size() == 1);

        double left = Run(node.GetChild(0));

        if (left == 0.0) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }

      case ASTNode::EQUALITY: {
        if (PRINT_DEBUG) std::cout << "Run: SUBTRACT case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        if (left == right) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }

      case ASTNode::NON_EQUALITY: {
        if (PRINT_DEBUG) std::cout << "Run: SUBTRACT case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        if (left == right) {
          return 0.0;
        } else {
          return 1.0;
        }

        break;
      }

      case ASTNode::LESS: {
        if (PRINT_DEBUG) std::cout << "Run: LESS case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        if (left < right) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }
      case ASTNode::LESS_EQUAL: {
        if (PRINT_DEBUG) std::cout << "Run: LESS_EQUAL case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        if (left <= right) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }
      case ASTNode::GREATER: {
        if (PRINT_DEBUG) std::cout << "Run: GREATER case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        if (left > right) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }

      case ASTNode::GREATER_EQUAL: {
        if (PRINT_DEBUG) std::cout << "Run: GREATER_EQUAL case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));
        double right = Run(node.GetChild(1));

        if (left >= right) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }

      case ASTNode::AND: {
        if (PRINT_DEBUG) std::cout << "Run: AND case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));

        if (left == 0.0) {
          return 0.0;
        }

        double right = Run(node.GetChild(1));

        if (left != 0.0 && right != 0.0) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }

      case ASTNode::OR: {
        if (PRINT_DEBUG) std::cout << "Run: OR case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));

        if (left != 0.0) {
          return 1.0;
        }

        double right = Run(node.GetChild(1));

        if (left != 0.0 || right != 0.0) {
          return 1.0;
        } else {
          return 0.0;
        }

        break;
      }
      case ASTNode::IF: {
        if (PRINT_DEBUG) std::cout << "Run: IF case" << std::endl;

        assert(node.GetChildren().size() == 2);

        double left = Run(node.GetChild(0));

        if (left != 0.0) {
          Run(node.GetChild(1));
        } else if (node.GetChildren().size() > 2) {
          Run(node.GetChild(2));
        }

        break;
      }

      case ASTNode::WHILE: {
        if (PRINT_DEBUG) std::cout << "Run: WHILE case" << std::endl;

        assert(node.GetChildren().size() == 2);

        while (Run(node.GetChild(0)) != 0.0) {
          Run(node.GetChild(1));
        }

        break;
      }
    }

    return 0.0;
  }

  void Run() { Run(root); }

  void PrintDebug(const ASTNode& node, std::string prefix = "") const {
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
      case ASTNode::PRINT_LITERAL:
        std::cout << "PRINT_LITERAL" << std::endl;
        break;

      case ASTNode::NEGATE:
        std::cout << "NEGATE" << std::endl;
        break;
      case ASTNode::LOGICAL_NOT:
        std::cout << "LOGICAL_NOT" << std::endl;
        break;
      case ASTNode::EXPONENT:
        std::cout << "EXPONENT" << std::endl;
        break;
      case ASTNode::AND:
        std::cout << "AND" << std::endl;
        break;
      case ASTNode::OR:
        std::cout << "OR" << std::endl;
        break;
      case ASTNode::EQUALITY:
        std::cout << "EQUALITY" << std::endl;
        break;
      case ASTNode::NON_EQUALITY:
        std::cout << "NON_EQUALITY" << std::endl;
        break;
      case ASTNode::LESS:
        std::cout << "LESS" << std::endl;
        break;
      case ASTNode::LESS_EQUAL:
        std::cout << "LESS_EQUAL" << std::endl;
        break;
      case ASTNode::GREATER:
        std::cout << "GREATER" << std::endl;
        break;
      case ASTNode::GREATER_EQUAL:
        std::cout << "GREATER_EQUAL" << std::endl;
        break;
      case ASTNode::ADD:
        std::cout << "ADD" << std::endl;
        break;
      case ASTNode::SUBTRACT:
        std::cout << "SUBTRACT" << std::endl;
        break;
      case ASTNode::MULT:
        std::cout << "MULT" << std::endl;
        break;
      case ASTNode::DIV:
        std::cout << "DIV" << std::endl;
        break;
      case ASTNode::MOD:
        std::cout << "MOD" << std::endl;
        break;
      case ASTNode::IF:
        std::cout << "IF" << std::endl;
        break;
      case ASTNode::WHILE:
        std::cout << "WHILE" << std::endl;
        break;
    }

    for (const auto& child : node.GetChildren()) {
      PrintDebug(child, prefix + "  ");
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

  if (PRINT_DEBUG) {
    std::cout << in_file.rdbuf();
  }

  MacroCalc calc(filename);

  // calc.PrintDebug();

  calc.Run();
}
