#include <cassert>
#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

class Shape {
public:
    [[nodiscard]] virtual double perimeter() const = 0;

    [[nodiscard]] virtual double area() const = 0;

    virtual ~Shape() = default;
};

class Triangle : public Shape {
private:
    double side1{};
    double side2{};
    double side3{};

public:
    Triangle(double side1, double side2, double side3)
        : side1(side1), side2(side2), side3(side3) {
    }

    [[nodiscard]] double perimeter() const final { return side1 + side2 + side3; }

    [[nodiscard]] double area() const final {
        double half_meter = perimeter() / 2;
        return std::sqrt(half_meter * (half_meter - side1) * (half_meter - side2) * (half_meter - side3));
    }
};


class Square final : public Shape {
private:
    double side{};

public:
    explicit Square(double side)
        : side(side) {
    }

    [[nodiscard]] double perimeter() const override { return 4 * side; }

    [[nodiscard]] double area() const override { return side * side; }
};


class Circle final : public Shape {
private:
    double radius{};

public:
    explicit Circle(double radius)
        : radius(radius) {
    }

    [[nodiscard]] double perimeter() const override { return 2 * std::numbers::pi * radius; }

    [[nodiscard]] double area() const override { return std::numbers::pi * radius * radius; }
};

int main() {
    {
        const auto * triangle = new Triangle(3, 4, 5);
        assert(std::abs(triangle->perimeter() - 12) < 1e-6);
        assert(std::abs(triangle->area() - 6) < 1e-6);
    }

    {
        const auto * square = new Square(5);
        assert(std::abs(square->perimeter() - 20) < 1e-6);
        assert(std::abs(square->area() - 25) < 1e-6);
    }

    {
        const auto * circle = new Circle(7);
        assert(std::abs(circle->perimeter() - 43.9823) < 1e-4);
        assert(std::abs(circle->area() - 153.938) < 1e-4);
    }

    {
        std::vector<Shape*> shapes;
        shapes.emplace_back(new Triangle(3, 4, 5));
        shapes.emplace_back(new Square(5));
        shapes.emplace_back(new Circle(7));

        for (const auto& shape : shapes) {
            assert(shape->perimeter() > 0);
            assert(shape->area() > 0);
        }
    }

    std::cout << "OK\n";
}