#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <vector>



class ASTNode {
public:
  // PLACE AST NODE INFO HERE.
  enum Type {
    EMPTY=0,
    VAR,
    PRINT,
    VAL,
    STATEMENT_BLOCK,
    ASSIGN,
    LITERAL,
    PRINT_LITERAL,

  };

private:
  Type type{EMPTY};
  double val{0.0};
  std::string literal = "";
  std::vector<ASTNode> children{};


public:
  // CONSTRUCTORS, ETC HERE.
  ASTNode(Type t) : type(t) { ; }

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

  // CODE TO ADD CHILDREN AND SETUP AST NODE HERE.
  void AddChild(ASTNode node) { 
    assert(node.NodeType() != EMPTY && children.size() < 2);
    children.push_back(node);
  }

  double GetVal() const {return val;}

  void SetVal(double num){
    val = num;
  }

  void SetLiteral(std::string s) { literal = s; }

  std::string GetLiteral() const { return literal; }

  


};
