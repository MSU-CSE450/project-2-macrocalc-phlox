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
  bool print_active = false;
  bool condition_active = false;
  std::string output_string;
  for (emplex::Token token: tokens)
  {
    if(token.id != -1){
      if(token.id == 252)
      {
        print_active = true;
      }
      else if(print_active and token.id == 245)
      {
        condition_active = true;
      }
      else if(token.id == 243)
      {
        condition_active = false;
      }
      else if(condition_active and token.id == 244)
      {
        for(auto ch : token.lexeme)
        {
          if(ch != '"'){
            output_string += ch;
          }
        }
      }
      else if(condition_active and token.id == 247)
      {
        output_string += token.lexeme;
      }
      else if(token.id == 246)
      {
        if(print_active)
        {
          print_active = false;
          std::cout << output_string << std::endl;
        }
      }
      
    }

  }

  // TO DO:  
  // PARSE input file to create Abstract Syntax Tree (AST).
  // EXECUTE the AST to run your program.
  
}
