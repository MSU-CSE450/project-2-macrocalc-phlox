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
    double var_val;
    size_t line_num;
  };
  // CODE TO STORE SCOPES AND VARIABLES HERE.
  
  // HINT: YOU CAN CONVERT EACH VARIABLE NAME TO A UNIQUE ID TO CLEANLY DEAL
  //       WITH SHADOWING AND LOOKING UP VARIABLES LATER.
  std::vector<VarData> var_info;
  using scope_t = std::unordered_map<std::string, size_t>;
  std::vector<std::unordered_map<std::string, size_t>> scope{1};//size_t represents ID, like 247

public:
  // CONSTRUCTOR, ETC HERE
  SymbolTable(){} 
  static constexpr size_t NO_ID = static_cast<size_t>(-1);

  // FUNCTIONS TO MANAGE SCOPES
  void IncScope() { scope.emplace_back(); }
  void DecScope() { scope.pop_back(); }
  void PushScope(std::unordered_map<std::string, size_t> mp) { scope.push_back(mp); }
  std::unordered_map<std::string, size_t> PopScope(){ 
    auto temp = scope.back(); 
    scope.pop_back(); 
    return temp;
  }
  
  // FUNCTIONS TO MANAGE VARIABLES
  size_t GetNumVars() const { return var_info.size(); }
  size_t GetVarID(std::string name) const {
    for (auto it = scope.rbegin(); it != scope.rend(); ++it){
      if (it->count(name)) return it->find(name)->second;
    }
    return NO_ID;
  }
  bool HasVar(std::string name) const { return (GetVarID(name) != NO_ID); }
  size_t AddVar(std::string name, size_t line_num) { 
    auto &curr_scope = scope.back();
    if (curr_scope.count(name)) {
      std::cerr << "ERROR (line " << line_num << "): Redeclaring variable '" << name << "'." << std::endl;
    }
    size_t var_id = var_info.size();
    var_info.emplace_back(name, line_num);
    curr_scope[name] = var_id;
    return var_id;
  }
  double GetValue(std::string name) const {
    assert(HasVar(name));
    return 0.0;
  }
  void SetValue(std::string name, double value) { 
    std::unordered_map<std::string, size_t> curr_map = PopScope();
    curr_map[name] = value;
    PushScope(curr_map);
  }
};
