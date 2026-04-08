#include "ast.hpp"
#include "utils.hpp"

#include <cmath>
#include <stdexcept>
#include <sstream>

static constexpr double EPS = 1e-12;

static bool is_zero(double x) {
    return std::fabs(x) < EPS;
}

static bool is_one(double x) {
    return std::fabs(x - 1.0) < EPS;
}

static bool is_minus_one(double x) {
    return std::fabs(x + 1.0) < EPS;
}

static void print_prefix(std::ostream& out, const std::string& prefix, bool is_last) {
    out << prefix;
    out << (is_last ? "\\-" : "|-");
}

static std::string next_prefix(const std::string& prefix, bool is_last) {
    return prefix + (is_last ? "  " : "| ");
}

NumberNode::NumberNode(double v) : value(v) {}

double NumberNode::eval(const std::map<std::string, double>&) const {
    return value;
}

NodePtr NumberNode::diff(const std::string&) const {
    return std::make_shared<NumberNode>(0.0);
}

NodePtr NumberNode::simplify() const {
    return std::make_shared<NumberNode>(value);
}

std::string NumberNode::str(int) const {
    return number_to_string(value);
}

int NumberNode::prec() const {
    return 100;
}

bool NumberNode::equals(const NodePtr& other) const {
    auto p = std::dynamic_pointer_cast<NumberNode>(other);
    return p && std::fabs(value - p->value) < EPS;
}

void NumberNode::pretty_print(std::ostream& out, const std::string& prefix, bool is_last) const {
    print_prefix(out, prefix, is_last);
    out << str() << "\n";
}

VariableNode::VariableNode(const std::string& n) : name(n) {}

double VariableNode::eval(const std::map<std::string, double>& vars) const {
    auto it = vars.find(name);
    if (it == vars.end()) {
        throw std::runtime_error("Unknown variable: " + name);
    }
    return it->second;
}

NodePtr VariableNode::diff(const std::string& var) const {
    return std::make_shared<NumberNode>(name == var ? 1.0 : 0.0);
}

NodePtr VariableNode::simplify() const {
    return std::make_shared<VariableNode>(name);
}

std::string VariableNode::str(int) const {
    return name;
}

int VariableNode::prec() const {
    return 100;
}

bool VariableNode::equals(const NodePtr& other) const {
    auto p = std::dynamic_pointer_cast<VariableNode>(other);
    return p && name == p->name;
}

void VariableNode::pretty_print(std::ostream& out, const std::string& prefix, bool is_last) const {
    print_prefix(out, prefix, is_last);
    out << name << "\n";
}

UnaryNode::UnaryNode(const std::string& o, NodePtr c) : op(o), child(std::move(c)) {}

double UnaryNode::eval(const std::map<std::string, double>& vars) const {
    double v = child->eval(vars);
    if (op == "+") {
        ensure_finite(v, "Domain error: unary plus");
        return v;
    }
    double r = -v;
    ensure_finite(r, "Domain error: unary minus");
    return r;
}

NodePtr UnaryNode::diff(const std::string& var) const {
    if (op == "+") {
        return child->diff(var);
    }
    return std::make_shared<UnaryNode>("-", child->diff(var));
}

NodePtr UnaryNode::simplify() const {
    NodePtr c = child->simplify();
    auto num = std::dynamic_pointer_cast<NumberNode>(c);

    if (num) {
        if (op == "+") {
            return std::make_shared<NumberNode>(num->value);
        }
        return std::make_shared<NumberNode>(-num->value);
    }

    auto un = std::dynamic_pointer_cast<UnaryNode>(c);
    if (op == "-" && un && un->op == "-") {
        return un->child->simplify();
    }

    if (op == "+") {
        return c;
    }
    return std::make_shared<UnaryNode>(op, c);
}

std::string UnaryNode::str(int parent_prec) const {
    std::string res = op + child->str(prec());
    if (prec() < parent_prec) {
        return "(" + res + ")";
    }
    return res;
}

int UnaryNode::prec() const {
    return 3;
}

bool UnaryNode::equals(const NodePtr& other) const {
    auto p = std::dynamic_pointer_cast<UnaryNode>(other);
    return p && op == p->op && child->equals(p->child);
}

void UnaryNode::pretty_print(std::ostream& out, const std::string& prefix, bool is_last) const {
    print_prefix(out, prefix, is_last);
    out << op << "\n";
    child->pretty_print(out, next_prefix(prefix, is_last), true);
}

FunctionNode::FunctionNode(const std::string& n, NodePtr a) : name(n), arg(std::move(a)) {}

double FunctionNode::eval(const std::map<std::string, double>& vars) const {
    double x = arg->eval(vars);

    if (name == "sin") {
        double r = std::sin(x);
        ensure_finite(r, "Domain error: sin");
        return r;
    }

    if (name == "cos") {
        double r = std::cos(x);
        ensure_finite(r, "Domain error: cos");
        return r;
    }

    if (name == "tan") {
        double r = std::tan(x);
        ensure_finite(r, "Domain error: tan");
        return r;
    }

    if (name == "asin") {
        if (x < -1.0 || x > 1.0) {
            throw std::runtime_error("Domain error: asin");
        }
        double r = std::asin(x);
        ensure_finite(r, "Domain error: asin");
        return r;
    }

    if (name == "acos") {
        if (x < -1.0 || x > 1.0) {
            throw std::runtime_error("Domain error: acos");
        }
        double r = std::acos(x);
        ensure_finite(r, "Domain error: acos");
        return r;
    }

    if (name == "atan") {
        double r = std::atan(x);
        ensure_finite(r, "Domain error: atan");
        return r;
    }

    if (name == "exp") {
        double r = std::exp(x);
        ensure_finite(r, "Domain error: exp");
        return r;
    }

    if (name == "log") {
        if (x <= 0.0) {
            throw std::runtime_error("Domain error: log");
        }
        double r = std::log(x);
        ensure_finite(r, "Domain error: log");
        return r;
    }

    if (name == "sqrt") {
        if (x < 0.0) {
            throw std::runtime_error("Domain error: sqrt");
        }
        double r = std::sqrt(x);
        ensure_finite(r, "Domain error: sqrt");
        return r;
    }

    throw std::runtime_error("Unknown function: " + name);
}

NodePtr FunctionNode::diff(const std::string& var) const {
    NodePtr d = arg->diff(var);

    if (name == "sin") {
        return std::make_shared<BinaryNode>("*",
            std::make_shared<FunctionNode>("cos", arg),
            d
        );
    }

    if (name == "cos") {
        return std::make_shared<BinaryNode>("*",
            std::make_shared<UnaryNode>("-",
                std::make_shared<FunctionNode>("sin", arg)
            ),
            d
        );
    }

    if (name == "tan") {
        return std::make_shared<BinaryNode>("*",
            std::make_shared<BinaryNode>("/",
                std::make_shared<NumberNode>(1.0),
                std::make_shared<BinaryNode>("^",
                    std::make_shared<FunctionNode>("cos", arg),
                    std::make_shared<NumberNode>(2.0)
                )
            ),
            d
        );
    }

    if (name == "asin") {
        return std::make_shared<BinaryNode>("/",
            d,
            std::make_shared<FunctionNode>("sqrt",
                std::make_shared<BinaryNode>("-",
                    std::make_shared<NumberNode>(1.0),
                    std::make_shared<BinaryNode>("^",
                        arg,
                        std::make_shared<NumberNode>(2.0)
                    )
                )
            )
        );
    }

    if (name == "acos") {
        return std::make_shared<UnaryNode>("-",
            std::make_shared<BinaryNode>("/",
                d,
                std::make_shared<FunctionNode>("sqrt",
                    std::make_shared<BinaryNode>("-",
                        std::make_shared<NumberNode>(1.0),
                        std::make_shared<BinaryNode>("^",
                            arg,
                            std::make_shared<NumberNode>(2.0)
                        )
                    )
                )
            )
        );
    }

    if (name == "atan") {
        return std::make_shared<BinaryNode>("/",
            d,
            std::make_shared<BinaryNode>("+",
                std::make_shared<NumberNode>(1.0),
                std::make_shared<BinaryNode>("^",
                    arg,
                    std::make_shared<NumberNode>(2.0)
                )
            )
        );
    }

    if (name == "exp") {
        return std::make_shared<BinaryNode>("*",
            std::make_shared<FunctionNode>("exp", arg),
            d
        );
    }

    if (name == "log") {
        return std::make_shared<BinaryNode>("/",
            d,
            arg
        );
    }

    if (name == "sqrt") {
        return std::make_shared<BinaryNode>("/",
            d,
            std::make_shared<BinaryNode>("*",
                std::make_shared<NumberNode>(2.0),
                std::make_shared<FunctionNode>("sqrt", arg)
            )
        );
    }

    throw std::runtime_error("Unknown function: " + name);
}

NodePtr FunctionNode::simplify() const {
    NodePtr a = arg->simplify();
    auto an = std::dynamic_pointer_cast<NumberNode>(a);

    if (an) {
        double x = an->value;

        if (name == "sin") {
            return std::make_shared<NumberNode>(std::sin(x));
        }

        if (name == "cos") {
            return std::make_shared<NumberNode>(std::cos(x));
        }

        if (name == "tan") {
            double r = std::tan(x);
            ensure_finite(r, "Domain error: tan");
            return std::make_shared<NumberNode>(r);
        }

        if (name == "asin") {
            if (x < -1.0 || x > 1.0) {
                throw std::runtime_error("Domain error: asin");
            }
            return std::make_shared<NumberNode>(std::asin(x));
        }

        if (name == "acos") {
            if (x < -1.0 || x > 1.0) {
                throw std::runtime_error("Domain error: acos");
            }
            return std::make_shared<NumberNode>(std::acos(x));
        }

        if (name == "atan") {
            return std::make_shared<NumberNode>(std::atan(x));
        }

        if (name == "exp") {
            double r = std::exp(x);
            ensure_finite(r, "Domain error: exp");
            return std::make_shared<NumberNode>(r);
        }

        if (name == "log") {
            if (x <= 0.0) {
                throw std::runtime_error("Domain error: log");
            }
            return std::make_shared<NumberNode>(std::log(x));
        }

        if (name == "sqrt") {
            if (x < 0.0) {
                throw std::runtime_error("Domain error: sqrt");
            }
            return std::make_shared<NumberNode>(std::sqrt(x));
        }
    }

    return std::make_shared<FunctionNode>(name, a);
}

std::string FunctionNode::str(int) const {
    return name + "(" + arg->str() + ")";
}

int FunctionNode::prec() const {
    return 100;
}

bool FunctionNode::equals(const NodePtr& other) const {
    auto p = std::dynamic_pointer_cast<FunctionNode>(other);
    return p && name == p->name && arg->equals(p->arg);
}

void FunctionNode::pretty_print(std::ostream& out, const std::string& prefix, bool is_last) const {
    print_prefix(out, prefix, is_last);
    out << name << "\n";
    arg->pretty_print(out, next_prefix(prefix, is_last), true);
}

BinaryNode::BinaryNode(const std::string& o, NodePtr l, NodePtr r)
    : op(o), left(std::move(l)), right(std::move(r)) {}

double BinaryNode::eval(const std::map<std::string, double>& vars) const {
    double a = left->eval(vars);
    double b = right->eval(vars);

    if (op == "+") {
        double r = a + b;
        ensure_finite(r, "Domain error: addition");
        return r;
    }

    if (op == "-") {
        double r = a - b;
        ensure_finite(r, "Domain error: subtraction");
        return r;
    }

    if (op == "*") {
        double r = a * b;
        ensure_finite(r, "Domain error: multiplication");
        return r;
    }

    if (op == "/") {
        if (is_zero(b)) {
            throw std::runtime_error("Division by zero");
        }
        double r = a / b;
        ensure_finite(r, "Domain error: division");
        return r;
    }

    if (op == "^") {
        if (is_zero(a) && b < 0.0) {
            throw std::runtime_error("Domain error: power");
        }

        double r = std::pow(a, b);
        ensure_finite(r, "Domain error: power");
        return r;
    }

    throw std::runtime_error("Unknown operator: " + op);
}

NodePtr BinaryNode::diff(const std::string& var) const {
    if (op == "+") {
        return std::make_shared<BinaryNode>("+",
            left->diff(var),
            right->diff(var)
        );
    }

    if (op == "-") {
        return std::make_shared<BinaryNode>("-",
            left->diff(var),
            right->diff(var)
        );
    }

    if (op == "*") {
        return std::make_shared<BinaryNode>("+",
            std::make_shared<BinaryNode>("*",
                left->diff(var),
                right
            ),
            std::make_shared<BinaryNode>("*",
                left,
                right->diff(var)
            )
        );
    }

    if (op == "/") {
        return std::make_shared<BinaryNode>("/",
            std::make_shared<BinaryNode>("-",
                std::make_shared<BinaryNode>("*",
                    left->diff(var),
                    right
                ),
                std::make_shared<BinaryNode>("*",
                    left,
                    right->diff(var)
                )
            ),
            std::make_shared<BinaryNode>("^",
                right,
                std::make_shared<NumberNode>(2.0)
            )
        );
    }

    if (op == "^") {
        auto rn = std::dynamic_pointer_cast<NumberNode>(right);

        if (rn) {
            return std::make_shared<BinaryNode>("*",
                std::make_shared<NumberNode>(rn->value),
                std::make_shared<BinaryNode>("*",
                    std::make_shared<BinaryNode>("^",
                        left,
                        std::make_shared<NumberNode>(rn->value - 1.0)
                    ),
                    left->diff(var)
                )
            );
        }

        return std::make_shared<BinaryNode>("*",
            std::make_shared<BinaryNode>("^", left, right),
            std::make_shared<BinaryNode>("+",
                std::make_shared<BinaryNode>("*",
                    right->diff(var),
                    std::make_shared<FunctionNode>("log", left)
                ),
                std::make_shared<BinaryNode>("*",
                    right,
                    std::make_shared<BinaryNode>("/",
                        left->diff(var),
                        left
                    )
                )
            )
        );
    }

    throw std::runtime_error("Unknown operator: " + op);
}

NodePtr BinaryNode::simplify() const {
    NodePtr l = left->simplify();
    NodePtr r = right->simplify();

    auto ln = std::dynamic_pointer_cast<NumberNode>(l);
    auto rn = std::dynamic_pointer_cast<NumberNode>(r);

    if (ln && rn) {
        double a = ln->value;
        double b = rn->value;

        if (op == "+") {
            double rr = a + b;
            ensure_finite(rr, "Domain error: addition");
            return std::make_shared<NumberNode>(rr);
        }

        if (op == "-") {
            double rr = a - b;
            ensure_finite(rr, "Domain error: subtraction");
            return std::make_shared<NumberNode>(rr);
        }

        if (op == "*") {
            double rr = a * b;
            ensure_finite(rr, "Domain error: multiplication");
            return std::make_shared<NumberNode>(rr);
        }

        if (op == "/") {
            if (is_zero(b)) {
                throw std::runtime_error("Division by zero");
            }
            double rr = a / b;
            ensure_finite(rr, "Domain error: division");
            return std::make_shared<NumberNode>(rr);
        }

        if (op == "^") {
            if (is_zero(a) && b < 0.0) {
                throw std::runtime_error("Domain error: power");
            }
            double rr = std::pow(a, b);
            ensure_finite(rr, "Domain error: power");
            return std::make_shared<NumberNode>(rr);
        }
    }

    if (op == "+") {
        if (ln && is_zero(ln->value)) {
            return r;
        }
        if (rn && is_zero(rn->value)) {
            return l;
        }
        if (l->equals(r)) {
            return std::make_shared<BinaryNode>("*",
                std::make_shared<NumberNode>(2.0),
                l
            )->simplify();
        }
    }

    if (op == "-") {
        if (rn && is_zero(rn->value)) {
            return l;
        }
        if (ln && is_zero(ln->value)) {
            return std::make_shared<UnaryNode>("-", r)->simplify();
        }
        if (l->equals(r)) {
            return std::make_shared<NumberNode>(0.0);
        }
    }

    if (op == "*") {
        if ((ln && is_zero(ln->value)) || (rn && is_zero(rn->value))) {
            return std::make_shared<NumberNode>(0.0);
        }
        if (ln && is_one(ln->value)) {
            return r;
        }
        if (rn && is_one(rn->value)) {
            return l;
        }
        if (ln && is_minus_one(ln->value)) {
            return std::make_shared<UnaryNode>("-", r)->simplify();
        }
        if (rn && is_minus_one(rn->value)) {
            return std::make_shared<UnaryNode>("-", l)->simplify();
        }
        if (l->equals(r)) {
            return std::make_shared<BinaryNode>("^",
                l,
                std::make_shared<NumberNode>(2.0)
            )->simplify();
        }
    }

    if (op == "/") {
        if (rn && is_one(rn->value)) {
            return l;
        }
        
    }

    if (op == "^") {
        if (rn && is_zero(rn->value)) {
            return std::make_shared<NumberNode>(1.0);
        }
        if (rn && is_one(rn->value)) {
            return l;
        }
        if (ln && is_one(ln->value)) {
            return std::make_shared<NumberNode>(1.0);
        }

        if (ln && is_zero(ln->value)) {
            if (rn && rn->value < 0.0) {
                throw std::runtime_error("Domain error: power");
            }
            if (rn && rn->value > 0.0) {
                return std::make_shared<NumberNode>(0.0);
            }
        }
    }

    return std::make_shared<BinaryNode>(op, l, r);
}

std::string BinaryNode::str(int parent_prec) const {
    std::string l = left->str(prec());
    std::string r = right->str(prec());

    if (left->prec() < prec() || (op == "^" && left->prec() == prec())) {
        l = "(" + left->str() + ")";
    }

    if (right->prec() < prec() ||
        ((op == "-" || op == "/" || op == "^") && right->prec() == prec())) {
        r = "(" + right->str() + ")";
    }

    std::string res = l + " " + op + " " + r;
    if (prec() < parent_prec) {
        return "(" + res + ")";
    }
    return res;
}

int BinaryNode::prec() const {
    if (op == "+" || op == "-") {
        return 1;
    }
    if (op == "*" || op == "/") {
        return 2;
    }
    return 4;
}

bool BinaryNode::equals(const NodePtr& other) const {
    auto p = std::dynamic_pointer_cast<BinaryNode>(other);
    return p && op == p->op && left->equals(p->left) && right->equals(p->right);
}

void BinaryNode::pretty_print(std::ostream& out, const std::string& prefix, bool is_last) const {
    print_prefix(out, prefix, is_last);
    out << op << "\n";

    std::string child_prefix = next_prefix(prefix, is_last);
    left->pretty_print(out, child_prefix, false);
    right->pretty_print(out, child_prefix, true);
}

NodePtr full_simplify(NodePtr node) {
    for (int i = 0; i < 20; ++i) {
        NodePtr next = node->simplify();
        if (next->str() == node->str()) {
            return next;
        }
        node = next;
    }
    return node;
}
