#include <iostream>
#include <source_location>

// #define NDEBUG

#ifdef NDEBUG
#define trace()
#else
#define trace() Tracer tracer##__LINE__
#endif

class Tracer {
public:
    explicit Tracer(std::source_location location = std::source_location::current())
        : location(location) {
        std::cout << location.function_name() << '\n';
    }

    ~Tracer() { std::cout << '~' << location.function_name() << '\n'; }

    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;
    Tracer(Tracer&&) = delete;
    Tracer& operator=(Tracer&&) = delete;

private:
    const std::source_location location;
};

void test1() {
    trace();
    std::cout << "test1 body\n";
}

void inner() {
    trace();
    std::cout << "inner body\n";
}

void outer() {
    trace();
    std::cout << "outer body\n";
    inner();
}

void early_return(int x) {
    trace();
    if (x == 0) {
        std::cout << "early return\n";
        return;
    }
    std::cout << "normal end\n";
}

void throwing() {
    trace();
    throw std::runtime_error("error");
}

int main() {
    test1();

    outer();

    early_return(0);
    early_return(1);

    try {
        throwing();
    } catch (...) {
        std::cout << "exception caught\n";
    }
}