#pragma once

#include <unordered_map>



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
    assert(id < var_info.size());
    return var_info[id].val;
  }

  void IncScope() { scope_stack.emplace_back(); }

  void DecScope() {
    assert(scope_stack.size() > 1);
    scope_stack.pop_back();
  }
};