#pragma once

#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>

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
  size_t GetNumVars() const { return var_info.size(); }
  size_t GetVarID(std::string name) const {
    for (auto it = scope.rbegin();
         it != scope.rend();
         ++it)
    {
      if (it->count(name)) return it->find(name)->second;
    }

    return NO_ID;
  }
  bool HasVar(std::string name) const { return (GetVarID(name) != NO_ID); }
  size_t AddVar(std::string name, size_t line_num) { 
    auto &curr_scope = scope.back();
    //if (curr_scope.count(name)) {
    //  std::cerr << "ERROR (line " << line_num << "): Redeclaring variable '" << name << "'." << std::endl;
    //}
    size_t var_id = var_info.size();
    var_info.emplace_back(VarData {name, line_num});
    curr_scope[name] = var_id;
    return var_id;
  }
  double GetValue(std::string name) const {
    assert(HasVar(name));
    return 0.0;
  }
  void SetValue(std::string name, double value) { 
    std::unordered_map<std::string, double> curr_map = PopScope();
    curr_map[name] = value;
    PushScope(curr_map);
  }
};
