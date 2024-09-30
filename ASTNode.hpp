#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "SymbolTable.hpp"

class ASTNode {
private:
  // PLACE AST NODE INFO HERE.
  enum Type {
    EMPTY=0,
    START_COND,
    END_COND,
    EXP,
    MULT,
    DIV,
    MOD,
    ADD,
    SUB,
    VAR
  };

  Type type{EMPTY};
  std::vector<ASTNode> children{};
  
public:
  // CONSTRUCTORS, ETC HERE.
  ASTNode(int t) : type(static_cast<Type>(t)) { ; }

  // CAN SPECIFY NODE TYPE AND ANY NEEDED VALUES HERE OR USING OTHER FUNCTIONS.

  // CODE TO ADD CHILDREN AND SETUP AST NODE HERE.
  void AddChild(ASTNode node) { ; }
  
  // CODE TO EXECUTE THIS NODE (AND ITS CHILDREN, AS NEEDED).
  double Run(SymbolTable & symbols) { ; }

};
