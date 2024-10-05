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


}
