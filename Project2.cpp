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

class MacroCalc {
  private:
    size_t token_id = 0;
    std::vector<emplex::Token> tokens;




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
