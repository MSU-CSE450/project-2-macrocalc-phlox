#pragma once

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>

/*
Needs two things: Name lookup and VarInfo
VarInfo:
  Keep a vector of variables - Vector<VarInfo> var_info
  Similar to how things were written in his SymbolTable
  Value of a variable is just a double
NameLookup:
  Use name to find variable value

Changing Scope:
  When you hit an open brace, increment scope
  When you hit a close brace, decrement scope
*/

class SymbolTable {
private:
  struct VarData{
    std::string name;
    size_t line_num;
  };
  // CODE TO STORE SCOPES AND VARIABLES HERE.
  
  // HINT: YOU CAN CONVERT EACH VARIABLE NAME TO A UNIQUE ID TO CLEANLY DEAL
  //       WITH SHADOWING AND LOOKING UP VARIABLES LATER.
  std::vector<VarData> var_info;
  std::vector<std::unordered_map<std::string, double>> scope{1};

public:
  // CONSTRUCTOR, ETC HERE
  SymbolTable(){} 
  static constexpr size_t NO_ID = static_cast<size_t>(-1);
  // FUNCTIONS TO MANAGE SCOPES
  void PushScope(std::unordered_map<std::string, double> mp) { scope.push_back(mp); }
  std::unordered_map<std::string, double> PopScope()
  { 
    auto temp = scope.back(); 
    scope.pop_back(); 
    return temp;
  }
  // FUNCTIONS TO MANAGE VARIABLES
  bool HasVar(std::string var_name) const {
    for (auto it = scope.rbegin(); it != scope.rend(); ++it) {
      if (it->find(var_name) != it->end()) {
        return true;
      }
    }
    return false;
  }
  size_t AddVar(std::string name) { 
    auto &curr_scope = scope.back();
    if (curr_scope.count(name)) {
      std::cerr << "ERROR"<< ": Redeclaring variable '" << name << "'." << std::endl;
    }
    size_t var_id = var_info.size();
    var_info.emplace_back(VarData {name});
    curr_scope[name] = var_id;
    return var_id;
  }
  double GetValue(std::string var_name) const {
    for (auto it = scope.rbegin(); it != scope.rend(); ++it) {
      if (it->find(var_name) != it->end()) {
        return it->at(var_name);
      }
    }
    return 0;
  }
  void SetValue(const std::string& var_name, double value) {
    for (auto it = scope.rbegin(); it != scope.rend(); ++it) {
      if (it->find(var_name) != it->end()) {
        (*it)[var_name] = value;
        return;
      }
    }
  }
  bool IsInMostRecentStack(std::string name)
  {
    for(auto it : scope.back())
    {
      if(it.first == name)
      {
        return true;
      }
    }
    return false;
  }
};

