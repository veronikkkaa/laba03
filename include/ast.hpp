#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

struct Node;
using NodePtr = std::shared_ptr<Node>;

struct Node {
    virtual ~Node() = default;

    virtual double eval(const std::map<std::string, double>& vars) const = 0;
    virtual NodePtr diff(const std::string& var) const = 0;
    virtual NodePtr simplify() const = 0;
    virtual std::string str(int parent_prec = 0) const = 0;
    virtual int prec() const = 0;
    virtual bool equals(const NodePtr& other) const = 0;

    virtual void pretty_print(std::ostream& out,
                              const std::string& prefix = "",
                              bool is_last = true) const = 0;
};

struct NumberNode : Node {
    double value;

    explicit NumberNode(double v);

    double eval(const std::map<std::string, double>& vars) const override;
    NodePtr diff(const std::string& var) const override;
    NodePtr simplify() const override;
    std::string str(int parent_prec = 0) const override;
    int prec() const override;
    bool equals(const NodePtr& other) const override;

    void pretty_print(std::ostream& out,
                      const std::string& prefix = "",
                      bool is_last = true) const override;
};

struct VariableNode : Node {
    std::string name;

    explicit VariableNode(const std::string& n);

    double eval(const std::map<std::string, double>& vars) const override;
    NodePtr diff(const std::string& var) const override;
    NodePtr simplify() const override;
    std::string str(int parent_prec = 0) const override;
    int prec() const override;
    bool equals(const NodePtr& other) const override;

    void pretty_print(std::ostream& out,
                      const std::string& prefix = "",
                      bool is_last = true) const override;
};

struct FunctionNode;
struct BinaryNode;

struct UnaryNode : Node {
    std::string op;
    NodePtr child;

    UnaryNode(const std::string& o, NodePtr c);

    double eval(const std::map<std::string, double>& vars) const override;
    NodePtr diff(const std::string& var) const override;
    NodePtr simplify() const override;
    std::string str(int parent_prec = 0) const override;
    int prec() const override;
    bool equals(const NodePtr& other) const override;

    void pretty_print(std::ostream& out,
                      const std::string& prefix = "",
                      bool is_last = true) const override;
};

struct FunctionNode : Node {
    std::string name;
    NodePtr arg;

    FunctionNode(const std::string& n, NodePtr a);

    double eval(const std::map<std::string, double>& vars) const override;
    NodePtr diff(const std::string& var) const override;
    NodePtr simplify() const override;
    std::string str(int parent_prec = 0) const override;
    int prec() const override;
    bool equals(const NodePtr& other) const override;

    void pretty_print(std::ostream& out,
                      const std::string& prefix = "",
                      bool is_last = true) const override;
};

struct BinaryNode : Node {
    std::string op;
    NodePtr left;
    NodePtr right;

    BinaryNode(const std::string& o, NodePtr l, NodePtr r);

    double eval(const std::map<std::string, double>& vars) const override;
    NodePtr diff(const std::string& var) const override;
    NodePtr simplify() const override;
    std::string str(int parent_prec = 0) const override;
    int prec() const override;
    bool equals(const NodePtr& other) const override;

    void pretty_print(std::ostream& out,
                      const std::string& prefix = "",
                      bool is_last = true) const override;
};

NodePtr full_simplify(NodePtr node);
