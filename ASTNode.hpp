#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "SymbolTable.hpp"

class ASTNode {
  public:
  enum Type {
    EMPTY=0,
    STATEMENT_BLOCK,
    START_COND,
    END_COND,
    EXP,
    MULT,
    DIV,
    MOD,
    ADD,
    SUB,
    ASSIGN,
    VAR,
    LITERAL,
    PRINT,

  };
private:
  // PLACE AST NODE INFO HERE.
  

  Type type{EMPTY};
  size_t val{0};
  std::vector<ASTNode> children{};

public:
  // CONSTRUCTORS, ETC HERE.
  ASTNode(int t) : type(static_cast<Type>(t)) { ; }

  ASTNode(Type type, ASTNode child) : type(type) { AddChild(child); }

  ASTNode(Type type, ASTNode child1, ASTNode child2)
    : type(type) { AddChild(child1); AddChild(child2); }

  ASTNode() = default;

  ASTNode(const ASTNode &) = default;

  ASTNode(ASTNode &&) = default;

  ASTNode & operator=(const ASTNode &) = default;
  
  ASTNode & operator=(ASTNode &&) = default;
  
  ~ASTNode() { }

  ASTNode(int t) : type(static_cast<Type>(t)) { ; }

  ASTNode(Type type, ASTNode child) : type(type) { AddChild(child); }

  ASTNode(Type type, ASTNode child1, ASTNode child2)
    : type(type) { AddChild(child1); AddChild(child2); }

  ASTNode() = default;

  ASTNode(const ASTNode &) = default;

  ASTNode(ASTNode &&) = default;

  ASTNode & operator=(const ASTNode &) = default;
  
  ASTNode & operator=(ASTNode &&) = default;
  
  ~ASTNode() { }

  // CAN SPECIFY NODE TYPE AND ANY NEEDED VALUES HERE OR USING OTHER FUNCTIONS.
  Type NodeType() const { return type; }

  std::vector<ASTNode> & GetChildren() { return children; }

  const std::vector<ASTNode> & GetChildren() const { return children; }

  ASTNode & GetChild(int pos) {
    assert(pos < children.size());
    return children[pos]; 
  }
  Type NodeType() const { return type; }

  std::vector<ASTNode> & GetChildren() { return children; }

  const std::vector<ASTNode> & GetChildren() const { return children; }

  ASTNode & GetChild(int pos) {
    assert(pos < children.size());
    return children[pos]; 
  }

  // CODE TO ADD CHILDREN AND SETUP AST NODE HERE.
  void AddChild(ASTNode node) { 
    assert(node.NodeType() != EMPTY && children.size() < 2);
    children.push_back(node);
  }

  size_t & GetVal() { return val; }

  const size_t & GetVal() const { return val; }

  void SetVal(size_t num){
    val = num;
  }
  void AddChild(ASTNode node) { 
    assert(node.NodeType() != EMPTY && children.size() < 2);
    children.push_back(node);
  }

  size_t & GetVal() { return val; }

  const size_t & GetVal() const { return val; }

  void SetVal(size_t num){
    val = num;
  }
  
  // CODE TO EXECUTE THIS NODE (AND ITS CHILDREN, AS NEEDED).
  double Run(SymbolTable & symbols) { ; }

};
