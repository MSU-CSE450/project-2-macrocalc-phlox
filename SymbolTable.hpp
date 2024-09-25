#pragma once

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>

class SymbolTable {
private:
  // CODE TO STORE SCOPES AND VARIABLES HERE.
  
  // HINT: YOU CAN CONVERT EACH VARIABLE NAME TO A UNIQUE ID TO CLEANLY DEAL
  //       WITH SHADOWING AND LOOKING UP VARIABLES LATER.
  std::vector<std::map<std::string, float>> scope;

public:
  // CONSTRUCTOR, ETC HERE
  SymbolTable(){} 
  // FUNCTIONS TO MANAGE SCOPES (NEED BODIES FOR THESE IF YOU WANT TO USE THEM)
  void PushScope(std::map<std::string, float> mp) { scope.push_back(mp); }
  std::map<std::string, float> PopScope()
  { 
    auto temp = scope.back(); 
    scope.pop_back(); 
    return temp;
  }

  // FUNCTIONS TO MANAGE VARIABLES - LOTS MORE NEEDED
  // (NEED REAL FUNCTION BODIES FOR THESE IF YOU WANT TO USE THEM)
  bool HasVar(std::string name) const { return false; }
  size_t AddVar(std::string name, size_t line_num) { return 0; }
  double GetValue(std::string name) const {
    assert(HasVar(name));
    return 0.0;
  }
  void SetValue(std::string name, double value) { ; }
};
