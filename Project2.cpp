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
  //ASTNode root{ASTNode::STATEMENT_BLOCK};

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

  // ASTNode MakeVarNode(const emplex::Token& token) {
  //   size_t var_id = symbols.GetVarID(token.lexeme);
  //   assert(var_id < symbols.GetNumVars());
  //   ASTNode out(ASTNode::VAR);
  //   out.SetVal(var_id);
  //   return out;
  // }

 public:
  MacroCalc(std::string filename) {  // Looked at WordLang.cpp for this
    std::ifstream file(filename);
    emplex::Lexer lexer;
    tokens = lexer.Tokenize(file);

    Parse();
  }
  bool scope_pushed = false;
  void Parse() {
    for(auto token : tokens)
    {
      ParseStatement();
    }
  }

  void ParseStatement() {
    switch (CurToken()) {
      using namespace emplex;
      case Lexer::ID_Print:
      {
        ParsePrint();
        break;
      }
      case Lexer::ID_Var:
      {
        ParseDeclare();
        break;
      }
      case Lexer::ID_Statement: {
        ParseIf();
        break;
      }
      
      case Lexer::ID_VariableName: {
        ParseNewVal();
        break;
      }
      // case Lexer::ID_WHILE: return ParseWhile();
      case Lexer::ID_StartScope:
      {
        ParseStatementBlock();
        break;
      }
      case Lexer::ID_Endscope:
      {
        break;
      }

      // case ';':
      //   return ASTNode{};
      default:
      {
        ParseExpression();
        break;
      }
    }
  }
  void ParseStatementBlock()
  {
    UseToken(emplex::Lexer::ID_StartScope);
    std::unordered_map<std::string, double> mp;
    symbols.PushScope(mp);
    while(CurToken() != emplex::Lexer::ID__EOF_ and CurToken() != emplex::Lexer::ID_Endscope)
    {
      ParseStatement();
    }
    if(CurToken() == emplex::Lexer::ID__EOF_ or CurToken() == emplex::Lexer::ID_Endscope)
    {
      symbols.PopScope();
      UseToken(emplex::Lexer::ID_Endscope);
    }
  }
  bool while_enabled = false;
  void ParseIf(){
    bool curr = false;
    auto type = UseToken(emplex::Lexer::ID_Statement);
    UseToken(emplex::Lexer::ID_StartCondition);
    if(type.lexeme == "if"){
      auto object = ParseExpression();
      auto comp = UseToken(emplex::Lexer::ID_Equivalent);
      if(comp.lexeme == "=="){
        auto object2 = UseToken();
        if(object == std::stod(object2.lexeme)){
          UseToken(emplex::Lexer::ID_EndCondition);
          ParseStatement();
        }
        else{
          while(CurToken().lexeme != ";"){
            UseToken();
          }
          UseToken(emplex::Lexer::ID_EOL);
        }
      }
    }
    else if(type.lexeme == "while"){
      //std::cout << ParseEquiv();
      UseToken(emplex::Lexer::ID_EndCondition);
      std::cout << CurToken().lexeme;
      while(ParseEquiv() == 1){
      }
    }
  }


  bool isScope = false;
  void ParsePrint() {
    UseToken(emplex::Lexer::ID_Print);
    UseToken(emplex::Lexer::ID_StartCondition);
    if (CurToken().id == emplex::Lexer::ID_LitString) {
        // Handle string output with variable replacement
        std::string output = "";
        std::string currvar = "";
        for(auto ch : CurToken().lexeme){
          if(ch != '"'){
            if(ch == '{')
            {
              isScope = true;
            }
            else if(ch == '}')
            {
              if(!symbols.HasVar(currvar))
              {
                Error(CurToken(), "Variable does not exist");
              }
              int i = std::to_string(symbols.GetValue(currvar)).size();
              std::string intermediate = std::to_string(symbols.GetValue(currvar));
              std::string last_dig = "";
              while(intermediate.back() == '0')
              {
                if(intermediate.back() == '0')
                {
                  intermediate.pop_back();
                }
              }
              last_dig = intermediate.back();
              if(last_dig == ".")
                {
                  intermediate.pop_back();
                }
              output += intermediate;
              
              currvar = "";
              isScope = false;
            }
            else if(isScope == false)
            {
              output += ch;
            }
            else
            {
              currvar += ch;
            }
          }
        }
        UseToken(emplex::Lexer::ID_LitString);
        std::cout << output << std::endl;
     } 
    else {
        // Handle expression output
        double value = ParseAnd();
        std::cout << value << std::endl;
    }
    UseToken(emplex::Lexer::ID_EndCondition);
    UseToken(emplex::Lexer::ID_EOL);  
  }

  void ParseDeclare() {
    UseToken(emplex::Lexer::ID_Var);  
    std::string varName = CurToken().lexeme;
    UseToken(emplex::Lexer::ID_VariableName);  
    if(!symbols.IsInMostRecentStack(varName)){
      symbols.AddVar(varName);
      if(CurToken().id == emplex::Lexer::ID_Equal) {
          UseToken(emplex::Lexer::ID_Equal);  
          double value = ParseExpression(); 
          symbols.SetValue(varName, value);  
      }
      else{
        Error(CurToken(), "left side must be variable");
      }
      UseToken(emplex::Lexer::ID_EOL);
    }
    else{
      Error(CurToken(), "Redeclaring Variable");
    }
  }

  double ParseExpression() {
    return ParseAddition();
  }

  double ParseAnd(){
    double left = ParseEquiv();
    while(CurToken().lexeme == "&&" or CurToken().lexeme == "||"){
      auto op = CurToken().lexeme;
      UseToken();
      double right = ParseEquiv();
      if(CurToken().lexeme == "&&"){
        if(left == 0 and right == 1) return 1;
        else return 0;
      }
      else if(CurToken().lexeme == "||"){
        if(left == 0 and right == 0) return 0;
        else return 1;
      }
    }
    return left;
  }
  
  double ParseEquiv() {
    if(CurToken().lexeme == "!"){
      UseToken(); 
      double right = ParseEquiv(); 
      return right == 0 ? 1 : 0;
    }
    double left = ParseAddition();
    while (CurToken().lexeme == "==" or CurToken().lexeme == "!=" or CurToken().lexeme == ">" or CurToken().lexeme == "<" or CurToken().lexeme == ">=" or CurToken().lexeme == "<=") {
      auto op = CurToken().lexeme;
      UseToken(CurToken().id);
      double right = ParseAddition();
      if (op == "==") {
        if(left == right) return 1;
        else return 0;
      } 
      else if(op == "!="){
        if(left != right) return 1;
        else return 0;
      }
      else if(op == ">"){
        if(left > right) return 1;
        else return 0;
      }
      else if(op == ">="){
        if(left >= right) return 1;
        else return 0;
      }
      else if(op == "<="){
        if(left <= right) return 1;
        else return 0;
      }
      else if(op == "<"){
        if(left < right) return 1;
        else return 0;
      }
    }
    return left;
  }

  void ParseNewVal()
  {
    std::string varName = CurToken().lexeme;
    UseToken(emplex::Lexer::ID_VariableName);  
    if(symbols.HasVar(varName))
    {
      if(CurToken().id == emplex::Lexer::ID_Equal) {
          UseToken(emplex::Lexer::ID_Equal);  
          double value = ParseExpression(); 
          symbols.SetValue(varName, value); 
          
      }
      else
      {
        Error(CurToken(), "left side must be variable");
      }
      UseToken(emplex::Lexer::ID_EOL);
    }
    else{
      Error(CurToken(), "Redeclaring Variable");
    }
  }
    // Parse additive expressions (e.g., addition and subtraction)
  double ParseAddition() {
    double left = ParseMult();
    while (CurToken().lexeme == "+" or CurToken().lexeme == "-") {
      auto op = CurToken().lexeme;
      UseToken(CurToken().id);  // Consume '+' or '-'
      double right = ParseMult();
      if (op == "+") {
        left += right;
      } else {
        left -= right;
      }
    }
    return left;
  }

  double ParseMult() {
    double left = ParsePrim();
    while (CurToken().lexeme == "*" or CurToken().lexeme == "/" or CurToken().lexeme == "**" or CurToken().lexeme == "%") {
      auto op = CurToken().lexeme;
      UseToken(CurToken().id);  // Consume '*', '/', or '**'
      double right = ParsePrim();
      if (op == "*") {
        left *= right;
      } 
      else if (op == "/") {
        if (right == 0) {
            Error(CurToken(),"Divide by zero");
        }
        left /= right;
      } 
      else if (op == "**") {
        left = pow(left, right);
      }
      else if (op == "%") {
        if (right == 0) {
          Error(CurToken(),"Divide by zero");
        }
        return fmod(left, right);
      } 
    }
    return left;
  }
  // Parse primary expressions (e.g., numbers, variables, or parenthesized expressions)
  double ParsePrim() {
    if (CurToken().lexeme == "-") {
        UseToken();  // Consume the '-'
        return -ParsePrim();  // Negate the next primary expression
    }
    if (CurToken().id == emplex::Lexer::ID_Value) {
      double value = std::stod(CurToken().lexeme);  // Convert token to double
      UseToken(emplex::Lexer::ID_Value);  // Consume the numeric value token
      return value;
    } 
    else if (CurToken().id == emplex::Lexer::ID_VariableName) {
      std::string varName = CurToken().lexeme;
      UseToken(emplex::Lexer::ID_VariableName);
      if (!symbols.HasVar(varName)) {
        Error(token_id, "Undefined variable: ", varName);
      }
      return symbols.GetValue(varName);  // Return the value of the variable
    } 
    else if (CurToken().id == emplex::Lexer::ID_StartCondition) {
      UseToken(emplex::Lexer::ID_StartCondition);
      double expr = ParseExpression();  // Parse expression inside parentheses
      UseToken(emplex::Lexer::ID_EndCondition);  // Expect closing parenthesis
      return expr;
    }
    else if(CurToken().id != emplex::Lexer::ID__EOF_){
      Error(token_id, "Unexpected token in primary expression: ", TokenName(CurToken().id));
    }
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

