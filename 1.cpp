#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class Operation {
public:
    virtual ~Operation() = default;
    virtual void execute(std::vector<double>& stack) const = 0;
};

class UnSin : public Operation {
public:
    void execute(std::vector<double> & stack) const override {
        if (stack.empty()) throw std::runtime_error("Empty stack (sin)");
        auto a = stack.back(); stack.pop_back();
        stack.push_back(sin(a));
    }
};

class UnCos : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.empty()) throw std::runtime_error("Empty stack (cos)");
        auto a = stack.back(); stack.pop_back();
        stack.push_back(cos(a));
    }
};

class UnTan : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.empty()) throw std::runtime_error("Empty stack (tg)");
        auto a = stack.back(); stack.pop_back();
        stack.push_back(tan(a));
    }
};

class UnExp : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.empty()) throw std::runtime_error("Empty stack (exp)");
        auto a = stack.back(); stack.pop_back();
        stack.push_back(exp(a));
    }
};

class UnLog : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.empty()) throw std::runtime_error("Empty stack (log)");
        auto a = stack.back(); stack.pop_back();
        if (a <= 0) throw std::runtime_error("Negative number (log)");
        stack.push_back(log(a));
    }
};

class UnSqrt : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.empty()) throw std::runtime_error("Empty stack (sqrt)");
        auto a = stack.back(); stack.pop_back();
        if (a < 0) throw std::runtime_error("Negative number (sqrt)");
        stack.push_back(sqrt(a));
    }
};

class BinPlus : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.size() < 2) throw std::runtime_error("Not enough operands (+)");
        auto a = stack.back(); stack.pop_back();
        auto b = stack.back(); stack.pop_back();
        stack.push_back(a + b);
    }
};

class BinMinus : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.size() < 2) throw std::runtime_error("Not enough operands (-)");
        auto a = stack.back(); stack.pop_back();
        auto b = stack.back(); stack.pop_back();
        stack.push_back(b - a);
    }
};

class BinMultiply : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.size() < 2) throw std::runtime_error("Not enough operands (*)");
        auto a = stack.back(); stack.pop_back();
        auto b = stack.back(); stack.pop_back();
        stack.push_back(a * b);
    }
};

class BinaryDivide : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.size() < 2) throw std::runtime_error("Not enough operands (/)");
        auto a = stack.back(); stack.pop_back();
        auto b = stack.back(); stack.pop_back();
        if (a == 0) throw std::runtime_error("Division by zero");
        stack.push_back(b / a);
    }
};

class BinaryAtan2 : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.size() < 2) throw std::runtime_error("Not enough operands (atan2)");
        auto a = stack.back(); stack.pop_back();
        auto b = stack.back(); stack.pop_back();
        stack.push_back(atan2(b, a));
    }
};

class BinaryPow : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.size() < 2) throw std::runtime_error("Not enough operands (pow)");
        auto a = stack.back(); stack.pop_back();
        auto b = stack.back(); stack.pop_back();
        stack.push_back(pow(b, a));
    }
};

class TernaryMedian : public Operation {
public:
    void execute(std::vector<double>& stack) const override {
        if (stack.size() < 3) throw std::runtime_error("Not enough operands (median)");
        auto a = stack.back(); stack.pop_back();
        auto b = stack.back(); stack.pop_back();
        auto c = stack.back(); stack.pop_back();
        
        std::vector temp = {a, b, c};
        std::ranges::sort(temp);
        
        stack.push_back(temp[1]);
    }
};

bool IsNum(const std::string& token) {
    std::istringstream iss(token);
    double value;
    return (iss >> value) && iss.eof();
}

class Calculator {
private:
    std::vector<double> stack;
    std::map<std::string, std::unique_ptr<Operation>> operations;
    
    void InitOperations() {
        operations["+"] = std::make_unique<BinPlus>();
        operations["-"] = std::make_unique<BinMinus>();
        operations["*"] = std::make_unique<BinMultiply>();
        operations["/"] = std::make_unique<BinaryDivide>();
        operations["atan2"] = std::make_unique<BinaryAtan2>();
        operations["pow"] = std::make_unique<BinaryPow>();
        
        operations["sin"] = std::make_unique<UnSin>();
        operations["cos"] = std::make_unique<UnCos>();
        operations["tg"] = std::make_unique<UnTan>();
        operations["exp"] = std::make_unique<UnExp>();
        operations["log"] = std::make_unique<UnLog>();
        operations["sqrt"] = std::make_unique<UnSqrt>();
        
        operations["median"] = std::make_unique<TernaryMedian>();
    }

public:
    Calculator() {
        InitOperations();
    }
    
    double evaluate(const std::string& expression) {
        while (!stack.empty()) stack.pop_back();
        
        std::istringstream iss(expression);
        std::string token;
        
        while (iss >> token) {
            if (IsNum(token)) {
                stack.push_back(std::stod(token));
            } 
            else {
                auto it = operations.find(token);
                if (it != operations.end()) {
                    it->second->execute(stack);
                } 
                else {
                    throw std::runtime_error("Unknown operation (" + token + ")");
                }
            }
        }

        if (stack.empty()) {
            throw std::runtime_error("Empty expression");
        }
        if (stack.size() > 1) {
            throw std::runtime_error("Values left in stack");
        }
        
        return stack.back();
    }
};

#include <iostream>
#include <cassert>

void test_calculator() {
    Calculator calc;

    std::cout << "=== Testing Calculator ===\n";

    std::cout << "1. Basic arithmetic:\n";
    assert(calc.evaluate("3 4 +") == 7.0);
    std::cout << "   3 4 + = 7 ✓\n";

    assert(calc.evaluate("10 5 -") == 5.0);
    std::cout << "   10 5 - = 5 ✓\n";

    assert(calc.evaluate("3 4 *") == 12.0);
    std::cout << "   3 4 * = 12 ✓\n";

    assert(calc.evaluate("20 4 /") == 5.0);
    std::cout << "   20 4 / = 5 ✓\n";

    std::cout << "\n2. Complex expressions:\n";
    assert(calc.evaluate("3 4 + 2 *") == 14.0);
    std::cout << "   3 4 + 2 * = 14 ✓\n";

    assert(calc.evaluate("5 1 2 + 4 * + 3 -") == 14.0);
    std::cout << "   5 1 2 + 4 * + 3 - = 14 ✓\n";

    std::cout << "\n3. Unary operations:\n";
    assert(fabs(calc.evaluate("0 sin") - 0.0) < 1e-10);
    std::cout << "   0 sin = 0 ✓\n";

    assert(fabs(calc.evaluate("0 cos") - 1.0) < 1e-10);
    std::cout << "   0 cos = 1 ✓\n";

    assert(fabs(calc.evaluate("1 exp") - exp(1.0)) < 1e-10);
    std::cout << "   1 exp = e ✓\n";

    assert(fabs(calc.evaluate("4 sqrt") - 2.0) < 1e-10);
    std::cout << "   4 sqrt = 2 ✓\n";

    std::cout << "\n4. Binary functions:\n";
    assert(fabs(calc.evaluate("2 3 pow") - 8.0) < 1e-10);
    std::cout << "   2 3 pow = 8 ✓\n";

    assert(fabs(calc.evaluate("1 1 atan2") - atan2(1, 1)) < 1e-10);
    std::cout << "   1 1 atan2 = π/4 ✓\n";

    std::cout << "\n5. Ternary functions:\n";
    assert(calc.evaluate("1 2 3 median") == 2.0);
    std::cout << "   1 2 3 median = 2 ✓\n";

    assert(calc.evaluate("5 1 4 median") == 4.0);
    std::cout << "   5 1 4 median = 4 ✓\n";

    std::cout << "\n6. Error handling:\n";

    try {
        calc.evaluate("");
        std::cout << "   Empty expression should throw ✗\n";
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "   Empty expression throws: \"" << e.what() << "\" ✓\n";
    }

    try {
        calc.evaluate("2 +");
        std::cout << "   '2 +' should throw ✗\n";
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "   '2 +' throws: \"" << e.what() << "\" ✓\n";
    }

    try {
        calc.evaluate("5 0 /");
        std::cout << "   '5 0 /' should throw ✗\n";
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "   '5 0 /' throws: \"" << e.what() << "\" ✓\n";
    }

    try {
        calc.evaluate("-1 sqrt");
        std::cout << "   '-1 sqrt' should throw ✗\n";
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "   '-1 sqrt' throws: \"" << e.what() << "\" ✓\n";
    }

    try {
        calc.evaluate("0 log");
        std::cout << "   '0 log' should throw ✗\n";
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "   '0 log' throws: \"" << e.what() << "\" ✓\n";
    }

    try {
        calc.evaluate("2 3 unknown");
        std::cout << "   Unknown operation should throw ✗\n";
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "   Unknown operation throws: \"" << e.what() << "\" ✓\n";
    }

    try {
        calc.evaluate("2 3 4 +");
        std::cout << "   Extra numbers should throw ✗\n";
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "   Extra numbers throws: \"" << e.what() << "\" ✓\n";
    }

    std::cout << "\n=== All tests passed! ===\n";
}


int main() {
    test_calculator();
}
