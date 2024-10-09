#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Below are some suggestions on how you might want to divide up your project.
// You may delete this and divide it up however you like.
#include "lexer.hpp"
#include "SymbolTable.hpp"
#include "ASTNode.hpp"

// class Parser {
//   private:
//     size_t token_id{0};
//     std::vector<emplex::Token> tokens;

//   public:
//     Parser(std::vector<emplex::Token> tokens) {
//       tokens = tokens;

//     }

//     emplex::Token CurrentToken() {return tokens[token_id]; }

//     ASTNode ParseStatement() // Taken from WordLang
//     { 
//         // Lexer
//         // switch (CurrentToken()) {
//         //   using namespace emplex;
//         //   case Lexer::ID_PRINT: return ParsePrint();
//         //   case Lexer::ID_TYPE: return ParseDeclare();
//         //   case Lexer::ID_FOREACH: return ParseForeach();
//         //   case ';': return ASTNode{0};
//         //   default:
//         //     return ParseExpression();
//         //}
//       }



  
// };

// // void Error(size_t line_num, std::string message) {                // https://github.com/MSU-CSE450/WordLang/blob/main/WordLang.cpp
// //   std::cerr << "ERROR (line " << line_num << "): " << message << std::endl;
// //   exit(1);
// // }





bool endsWith(const std::string& fullString, const std::string& ending) //https://www.geeksforgeeks.org/check-if-string-ends-substring-in-cpp/
{
    // Check if the ending string is longer than the full
    // string
    if (ending.size() > fullString.size())
        return false;

    // Compare the ending of the full string with the target
    // ending
    return fullString.compare(fullString.size() - ending.size(), ending.size(), ending) == 0;
}

// template <typename... Ts>
// void Error(size_t line_num, Ts... message) {
//   std::cerr << "ERROR (line " << line_num << "): ";
//   (std::cerr << ... << message);
//   std::cerr << std::endl;
//   exit(1);
// }
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
  std::ifstream input(filename);
  emplex::Lexer lexer;
  std::vector<emplex::Token> tokens = lexer.Tokenize(input);
  //auto symbol_table = SymbolTable();
  std::string output_string;
  bool print_active = false;
  bool condition_active = false;
  std::string current_variable = "";
  bool value_change = false;
  std::unordered_map<std::string, std::string> mp;
  bool equal = false;
  bool statement = false;
  for (emplex::Token token: tokens)
  {
    if(token.id != -1){
      if(token.id == 252 && !print_active) //ID Print detected
      {
        print_active = true; //says print is active
      }
      else if(token.id == 255 && !statement)
      {
        statement = true;
      }
      else if(statement == true)
      {
        if(token.id == 255)
        {
          std::cerr << "cannot put conditional within condtitional" << std::endl;
          exit(1);
        }
      }
      else if(print_active && token.id == 245) //detects open parentheses for print statement
      {
        condition_active = true;
      }
      else if(token.id == 243) //detects close parentheses
      {
        condition_active = false;
      }
      else if(condition_active && token.id == 244 && print_active) //if printing a literal string
      {
        for(auto ch : token.lexeme)
        {
          if(ch != '"'){
            output_string += ch;
          }
        }
      }
      else if(condition_active && token.id == 247 && print_active) //if printing number
      {
        output_string += token.lexeme;
      }
      else if(condition_active && token.id == 249 && print_active) //if printing a variable
      {
        output_string += mp[token.lexeme];
        current_variable = token.lexeme;
      }
      else if(condition_active && token.id == 241) //if an equation in print statement
      {
        //AST(token.lexeme, mp);
        std::cout << current_variable;
      }
      else if(token.id == 246 && condition_active == false) //semicolon (end of line) detected 
      {
        current_variable = "";
        value_change = false;
        if(print_active) //if print statement
        {
          print_active = false;
          std::cout << output_string << std::endl;
          condition_active = false;
          output_string = "";
        }
      }
      else if(print_active && token.id == 252)
      {
        std::cerr << std::endl;
        exit(1);
      }
      else if(token.id == 251) //var keyword detected
      {
        value_change = true; //indicates value is being changed
      }
      else if(value_change == true) //if a value change is ongoing
      {
        if(token.id == 249 && current_variable == "") // if a variable name is detected
        {
          current_variable = token.lexeme; //current variable is set
        }
        else if(token.id == 249 && current_variable != "" && equal)
        {
          mp[current_variable] = mp[token.lexeme];
        }
        else if (token.id == 248)
        {
          equal = true;
        }
        else if(token.id == 249 && !equal)
        {
          std::cerr << "AAAAAA" << std::endl;
          exit(1);
        }
        else if(token.id == 247 && current_variable == "")
        {
          std::cerr << "You cannot make a variable name a number" << std::endl;
          exit(1);
        }
        else if(token.id == 247 && current_variable != "" && equal) //if a value is detected
        {
          std::string extra = ".0";
          if(endsWith(token.lexeme, extra)) //if value ends with ".0"
          {
            token.lexeme.pop_back(); // removing trailing 0
            token.lexeme.pop_back(); // remove decimal
          }
          mp[current_variable] = token.lexeme; //sets map value to token.lexeme
        }
      }
    }
    // if(token.id == 245) //If start condition active
    // {
    //   //auto A = ASTNode(1);
    //   //std::cout << A.NodeType() << std::endl;
    // }
  /*
  
  */
  // TO DO:  
  // PARSE input file to create Abstract Syntax Tree (AST).
  // EXECUTE the AST to run your program.
}
}
