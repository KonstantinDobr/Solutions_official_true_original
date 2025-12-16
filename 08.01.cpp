#include <iostream>
#include <limits>
#include <cassert>
#include <cmath>

union FloatUnion {
    float f;
    unsigned int u;
};

int LogInt(int n) {
    auto uns_n = static_cast<unsigned int>(n);
    int log = -1;

    while (uns_n > 0) {
        uns_n >>= 1;
        ++log;
    }
    return log;
}

int integer_log2_float(float x) {
    FloatUnion temp{};
    temp.f = x;
    unsigned int uns_x = temp.u;

    unsigned int exp = (uns_x >> 23) & ((1u << 8) - 1);
    unsigned int mantissa = uns_x & ((1u << 23) - 1);

    if (exp == ((1u << 8) - 1)) {
        return (mantissa == 0) ?
               std::numeric_limits<int>::max() :
               std::numeric_limits<int>::min();
    }

    if (exp == 0 && mantissa == 0) {
        return std::numeric_limits<int>::min();
    }

    if (exp == 0) {
        int pos = 0;
        unsigned int m = mantissa;

        while (m > 0) {
            m >>= 1;
            ++pos;
        }
        return -150 + pos;
    }

    return static_cast<int>(exp) - 127;
}

void test_LogInt() {
    std::cout << "=== Testing LogInt ===" << std::endl;

    assert(LogInt(1) == 0);
    assert(LogInt(2) == 1);
    assert(LogInt(3) == 1);
    assert(LogInt(4) == 2);
    assert(LogInt(7) == 2);
    assert(LogInt(8) == 3);
    assert(LogInt(16) == 4);
    assert(LogInt(1024) == 10);
    assert(LogInt(1 << 20) == 20);
    assert(LogInt((1 << 30) - 1) == 29);
    assert(LogInt(std::numeric_limits<int>::max()) == 30);

    std::cout << "All LogInt tests passed!" << std::endl;
}

void test_LogFloat() {
    std::cout << "\n=== Testing integer_log2_float ===" << std::endl;

    assert(integer_log2_float(1.0f) == 0);
    assert(integer_log2_float(2.0f) == 1);
    assert(integer_log2_float(4.0f) == 2);
    assert(integer_log2_float(8.0f) == 3);
    assert(integer_log2_float(1.5f) == 0);
    assert(integer_log2_float(3.0f) == 1);
    assert(integer_log2_float(0.0f) == std::numeric_limits<int>::min());

    float pos_inf = std::numeric_limits<float>::infinity();
    float neg_inf = -pos_inf;
    float nan = std::numeric_limits<float>::quiet_NaN();

    assert(integer_log2_float(pos_inf) == std::numeric_limits<int>::max());
    assert(integer_log2_float(nan) == std::numeric_limits<int>::min());

    float smallest_denormal = std::numeric_limits<float>::denorm_min();
    assert(integer_log2_float(smallest_denormal) == -149);

    float min_normal = std::numeric_limits<float>::min();
    float largest_denormal = std::nextafterf(min_normal, 0.0f);
    int log_largest_denormal = integer_log2_float(largest_denormal);
    assert(log_largest_denormal == -127 || log_largest_denormal == -128);

    assert(integer_log2_float(min_normal) == -126);

    float max_float = std::numeric_limits<float>::max();
    assert(integer_log2_float(max_float) == 127);

    std::cout << "All LogFloat tests passed!" << std::endl;
}

int main() {
    test_LogInt();
    test_LogFloat();
}
