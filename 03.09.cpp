#include <array>
#include <cstdint>
#include <iostream>
#include <sstream>

class IPv4 {
private:
    std::array<std::uint8_t, 4> blocks_ = {0, 0, 0, 0};

public:
    IPv4() = default;

    explicit IPv4(std::array<std::uint8_t, 4> blocks_) : blocks_(blocks_) {}

    explicit IPv4(std::uint8_t p1, std::uint8_t p2, std::uint8_t p3, std::uint8_t p4) : blocks_({p1, p2, p3, p4}) {}

    IPv4(const IPv4 &other) = default;

    IPv4 &operator++() {
        for (int i = 3; i >= 0; --i) {
            if (blocks_[i] != 255) {
                ++blocks_[i];
                return *this;
            }
            blocks_[i] = 0;
        }
        return *this;
    }

    IPv4 operator++(int) {
        IPv4 copy = *this;
        ++*this;
        return copy;
    }

    IPv4 &operator--() {
        for (int i = 3; i >= 0; --i) {
            if (blocks_[i] != 0) {
                --blocks_[i];
                return *this;
            }
            blocks_[i] = 255;
        }
        return *this;
    }

    IPv4 operator--(int) {
        IPv4 copy = *this;
        --*this;
        return copy;
    }

    friend bool operator==(const IPv4 &lhs, const IPv4 &rhs) { return lhs.blocks_ == rhs.blocks_; }

    friend bool operator<=(const IPv4 &lhs, const IPv4 &rhs) { return lhs.blocks_ <= rhs.blocks_; }

    friend bool operator>=(const IPv4 &lhs, const IPv4 &rhs) { return lhs.blocks_ >= rhs.blocks_; }

    friend bool operator!=(const IPv4 &lhs, const IPv4 &rhs) { return lhs.blocks_ != rhs.blocks_; }

    friend bool operator<(const IPv4 &lhs, const IPv4 &rhs) { return lhs.blocks_ < rhs.blocks_; }

    friend bool operator>(const IPv4 &lhs, const IPv4 &rhs) { return lhs.blocks_ > rhs.blocks_; }

    friend std::ostream &operator<<(std::ostream &os, const IPv4 &ip) {
        std::stringstream ss;
        ss << static_cast<int>(ip.blocks_[0]) << '.'
           << static_cast<int>(ip.blocks_[1]) << '.'
           << static_cast<int>(ip.blocks_[2]) << '.'
           << static_cast<int>(ip.blocks_[3]);
        return os << ss.str();
    }

    friend std::istream &operator>>(std::istream &is, IPv4 &ip) {
        std::string line;
        std::getline(is, line);

        std::stringstream ss(line);

        int p1{};
        int p2{};
        int p3{};
        int p4{};
        char dot{};

        ss >> p1 >> dot >> p2 >> dot >> p3 >> dot >> p4;
        ip.blocks_ = {static_cast<uint8_t>(p1), static_cast<uint8_t>(p2), static_cast<uint8_t>(p3),
                     static_cast<uint8_t>(p4)};
        return is;
    }
};

void main_test() {
    IPv4 ip1(192, 168, 1, 1);
    IPv4 ip2(std::array<uint8_t, 4>{10, 0, 0, 1});

    std::cout << "ip1: " << ip1 << std::endl;
    std::cout << "ip2: " << ip2 << std::endl;

    ++ip1;
    std::cout << "++ip1: " << ip1 << std::endl;

    IPv4 ip3 = ip2++;
    std::cout << "ip2++: " << ip2 << ", old: " << ip3 << std::endl;

    --ip1;
    std::cout << "--ip1: " << ip1 << std::endl;

    IPv4 ip4 = ip2--;
    std::cout << "ip2--: " << ip2 << ", old: " << ip4 << std::endl;

    std::cout << "ip1 == ip2: " << (ip1 == ip2) << std::endl;
    std::cout << "ip1 != ip2: " << (ip1 != ip2) << std::endl;
    std::cout << "ip1 < ip2: " << (ip1 < ip2) << std::endl;
    std::cout << "ip1 > ip2: " << (ip1 > ip2) << std::endl;
}

void input_test() {
    IPv4 ip5;
    std::cin >> ip5;
    std::cout << ip5 << std::endl;
}

int main() {
    main_test();
    input_test();
}
