#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>

const double eps = 1e-9;

class Vector {
private:
    struct SharedData {
        double* data;
        size_t size;
        int ref_cnt;

        SharedData(size_t sz) : size(sz), ref_cnt(1) {
            data = new double[sz];
        }

        ~SharedData() {
            delete[] data;
        }

        void add_ref() { ++ref_cnt; }
        void release() { if (--ref_cnt == 0) delete this; }
    };

    SharedData* shared_;
    size_t offset_;
    size_t size_;
    mutable int const_lock_count_;

    void copy_on_write() {
        SharedData* new_shared = new SharedData(size_);
        std::copy(shared_->data + offset_,
                  shared_->data + offset_ + size_,
                  new_shared->data);
        shared_->release();
        shared_ = new_shared;
        offset_ = 0;
    }

public:
    explicit Vector(size_t size)
        : shared_(new SharedData(size)), offset_(0), size_(size), const_lock_count_(0) {}

    Vector(const Vector& other)
        : shared_(other.shared_), offset_(other.offset_), size_(other.size_),
          const_lock_count_(0) {
        shared_->add_ref();
    }

    Vector(const Vector& other, size_t offset, size_t size)
        : shared_(other.shared_), offset_(other.offset_ + offset), size_(size),
          const_lock_count_(0) {
        assert(offset + size <= other.size_);
        shared_->add_ref();
    }

    ~Vector() {
        shared_->release();
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            shared_->release();
            shared_ = other.shared_;
            offset_ = other.offset_;
            size_ = other.size_;
            shared_->add_ref();
            const_lock_count_ = 0;
        }
        return *this;
    }

    size_t size() const { return size_; }

    const double& operator[](size_t index) const {
        assert(index < size_);
        return shared_->data[offset_ + index];
    }

    double& operator[](size_t index) {
        assert(index < size_);
        assert(const_lock_count_ == 0);

        if (shared_->ref_cnt > 1) copy_on_write();

        return shared_->data[offset_ + index];
    }

    const double* AcquireConstBuffer() const {
        ++const_lock_count_;
        return shared_->data + offset_;
    }

    void ReleaseConstBuffer() const {
        assert(const_lock_count_ > 0);
        --const_lock_count_;
    }

    double* AcquireBuffer() {
        assert(const_lock_count_ == 0);

        if (shared_->ref_cnt > 1) copy_on_write();

        return shared_->data + offset_;
    }

    class ConstBuffer {
    public:
        explicit ConstBuffer(const Vector& vec) : vec_(vec) {
            data_ = vec_.AcquireConstBuffer();
        }

        ~ConstBuffer() {
            vec_.ReleaseConstBuffer();
        }

        const double* operator->() const { return data_; }

        ConstBuffer(const ConstBuffer&) = delete;
        ConstBuffer& operator=(const ConstBuffer&) = delete;

    private:
        const Vector& vec_;
        const double* data_;
    };

    class Buffer {
    public:
        explicit Buffer(Vector& vec) : vec_(vec) {
            data_ = vec_.AcquireBuffer();
        }

        ~Buffer() = default;

        double* operator->() const { return data_; }

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

    private:
        Vector& vec_;
        double* data_;
    };

    ConstBuffer GetConstBuffer() const {
        return ConstBuffer(*this);
    }

    Buffer GetBuffer() {
        return Buffer(*this);
    }
};

void test_basic_creation_and_access() {
    std::cout << "Test 1: Basic creation and access... ";
    Vector v1(5);
    assert(v1.size() == 5);

    v1[0] = 1.0;
    v1[1] = 2.0;
    v1[2] = 3.0;
    assert(v1[0] == 1.0);
    assert(v1[1] == 2.0);
    assert(v1[2] == 3.0);
    std::cout << "OK\n";
}

void test_copy_on_write() {
    std::cout << "Test 2: Copy-on-write on copying... ";
    Vector v1(3);
    v1[0] = 10.0;
    v1[1] = 20.0;
    v1[2] = 30.0;

    Vector v2 = v1;
    assert(v2[0] == 10.0);
    assert(v2[1] == 20.0);
    assert(v2[2] == 30.0);

    v2[0] = 100.0;
    assert(v2[0] == 100.0);
    assert(v1[0] == 10.0);

    assert(v1[1] == 20.0);
    assert(v2[1] == 20.0);
    std::cout << "OK\n";
}

void test_subvectors() {
    std::cout << "Test 3: Subvectors... ";
    Vector v1(10);
    for (size_t i = 0; i < 10; ++i) {
        v1[i] = static_cast<double>(i);
    }

    // Создаем подвектор
    Vector v2(v1, 3, 4);
    assert(v2.size() == 4);
    assert(v2[0] == 3.0);
    assert(v2[1] == 4.0);
    assert(v2[2] == 5.0);
    assert(v2[3] == 6.0);

    // Изначально они делят данные - проверяем
    assert(v1[3] == 3.0);

    // Изменяем подвектор - должен сработать COW
    v2[0] = 300.0;
    assert(v2[0] == 300.0);  // Подвектор изменился
    assert(v1[3] == 3.0);    // Оригинал НЕ изменился (COW сработал!)

    // Создаем копию подвектора
    Vector v3 = v2;

    // Изначально v2 и v3 делят данные
    assert(v2[1] == 4.0);
    assert(v3[1] == 4.0);

    // Изменяем v3 - должен сработать COW
    v3[1] = 400.0;
    assert(v3[1] == 400.0);  // v3 изменился
    assert(v2[1] == 4.0);    // v2 НЕ изменился (COW сработал!)
    assert(v1[4] == 4.0);    // v1 тоже НЕ изменился
    std::cout << "OK\n";
}

void test_nested_subvectors() {
    std::cout << "Test 9: Nested subvectors... ";
    Vector v1(10);
    for (size_t i = 0; i < 10; ++i) {
        v1[i] = static_cast<double>(i);
    }

    // Создаем подвектор от 2 до 7
    Vector v2(v1, 2, 6);
    // Создаем подвектор от подвектора
    Vector v3(v2, 1, 3);

    assert(v3.size() == 3);
    assert(v3[0] == 3.0);  // v1[3]
    assert(v3[1] == 4.0);  // v1[4]
    assert(v3[2] == 5.0);  // v1[5]

    // Изначально v1, v2, v3 делят данные
    assert(v1[3] == 3.0);
    assert(v2[1] == 3.0);
    assert(v3[0] == 3.0);

    // Изменяем v3 - должен сработать COW
    v3[0] = 300.0;
    assert(v3[0] == 300.0);  // v3 изменился
    assert(v2[1] == 3.0);    // v2 НЕ изменился (COW сработал!)
    assert(v1[3] == 3.0);    // v1 НЕ изменился
    std::cout << "OK\n";
}

void test_assignment_operator() {
    std::cout << "Test 4: Assignment operator... ";
    Vector v1(3);
    v1[0] = 1.0;
    v1[1] = 2.0;
    v1[2] = 3.0;

    Vector v2(2);
    v2 = v1;

    assert(v2.size() == 3);
    assert(v2[0] == 1.0);
    assert(v2[1] == 2.0);
    assert(v2[2] == 3.0);

    v2[0] = 100.0;
    assert(v2[0] == 100.0);
    assert(v1[0] == 1.0);
    std::cout << "OK\n";
}

void test_const_buffer() {
    std::cout << "Test 5: ConstBuffer (read)... ";
    Vector v1(5);
    for (size_t i = 0; i < 5; ++i) {
        v1[i] = static_cast<double>(i * 10);
    }

    {
        Vector::ConstBuffer buf = v1.GetConstBuffer();
        const double* data = buf.operator->();

        assert(data[0] == 0.0);
        assert(data[1] == 10.0);
        assert(data[2] == 20.0);
        assert(data[3] == 30.0);
        assert(data[4] == 40.0);
    }

    v1[0] = 100.0;
    assert(v1[0] == 100.0);
    std::cout << "OK\n";
}

void test_buffer_write() {
    std::cout << "Test 6: Buffer (write)... ";
    Vector v1(4);
    for (int i = 0; i < 4; ++i) v1[i] = 0.0;
    Vector v2 = v1;

    {
        Vector::Buffer buf = v1.GetBuffer();
        double* data = buf.operator->();

        data[0] = 1.0;
        data[1] = 2.0;
        data[2] = 3.0;
        data[3] = 4.0;
    }

    assert(v1[0] == 1.0);
    assert(v1[1] == 2.0);
    assert(v1[2] == 3.0);
    assert(v1[3] == 4.0);

    assert(v2[0] == 0.0);
    assert(v2[1] == 0.0);
    assert(v2[2] == 0.0);
    assert(v2[3] == 0.0);
    std::cout << "OK\n";
}

void test_multiple_copies_and_cow() {
    std::cout << "Test 7: Multiple copies and COW... ";
    Vector original(5);
    for (size_t i = 0; i < 5; ++i) {
        original[i] = static_cast<double>(i + 1);
    }

    Vector copy1 = original;
    Vector copy2 = original;
    Vector copy3 = original;

    assert(copy1[2] == 3.0);
    assert(copy2[2] == 3.0);
    assert(copy3[2] == 3.0);

    copy1[2] = 300.0;
    assert(copy1[2] == 300.0);
    assert(copy2[2] == 3.0);
    assert(copy3[2] == 3.0);
    assert(original[2] == 3.0);

    copy2[2] = 200.0;
    assert(copy2[2] == 200.0);
    assert(copy1[2] == 300.0);
    assert(copy3[2] == 3.0);
    assert(original[2] == 3.0);
    std::cout << "OK\n";
}

void test_direct_acquire_methods() {
    std::cout << "Test 8: Direct AcquireConstBuffer and AcquireBuffer... ";
    Vector v1(3);
    v1[0] = 1.5;
    v1[1] = 2.5;
    v1[2] = 3.5;

    const double* read_ptr = v1.AcquireConstBuffer();
    assert(read_ptr[0] == 1.5);
    assert(read_ptr[1] == 2.5);
    assert(read_ptr[2] == 3.5);
    v1.ReleaseConstBuffer();

    Vector v2 = v1;
    double* write_ptr = v1.AcquireBuffer();
    write_ptr[0] = 100.5;

    assert(v1[0] == 100.5);
    assert(v2[0] == 1.5);
    std::cout << "OK\n";
}

void test_self_assignment() {
    std::cout << "Test 10: Self-assignment... ";
    Vector v1(4);
    v1[0] = 1.0;
    v1[1] = 2.0;
    v1[2] = 3.0;
    v1[3] = 4.0;

    v1 = v1;

    assert(v1.size() == 4);
    assert(v1[0] == 1.0);
    assert(v1[1] == 2.0);
    assert(v1[2] == 3.0);
    assert(v1[3] == 4.0);
    std::cout << "OK\n";
}

void test_buffer_transfers() {
    std::cout << "Test 11: Buffer transfers... ";
    Vector v1(4);
    Vector v2(4);

    for (size_t i = 0; i < 4; ++i) {
        v1[i] = static_cast<double>(i + 1);
    }

    v2 = v1;

    for (size_t i = 0; i < 4; ++i) {
        v2[i] = static_cast<double>((i + 1) * 10);
    }

    assert(v1[0] == 1.0 && v2[0] == 10.0);
    assert(v1[1] == 2.0 && v2[1] == 20.0);
    assert(v1[2] == 3.0 && v2[2] == 30.0);
    assert(v1[3] == 4.0 && v2[3] == 40.0);
    std::cout << "OK\n";
}

void test_const_correctness() {
    std::cout << "Test 12: Const correctness... ";
    const Vector v1(3);

    double val = v1[0];
    assert(std::abs(val) < eps);

    Vector::ConstBuffer buf = v1.GetConstBuffer();
    const double* data = buf.operator->();
    assert(data != nullptr);

    // Эти строки не должны компилироваться:
    // v1[0] = 10.0;
    // v1.AcquireBuffer();
    std::cout << "OK\n";
}

void test_edge_cases() {
    std::cout << "Test 13: Edge cases... ";
    // Тест с нулевым размером
    Vector v1(0);
    assert(v1.size() == 0);

    // Тест с подвектором нулевого размера
    Vector v2(5);
    Vector v3(v2, 2, 0);
    assert(v3.size() == 0);

    // Тест с подвектором, занимающим весь оригинальный вектор
    Vector v4(5);
    for (size_t i = 0; i < 5; ++i) {
        v4[i] = static_cast<double>(i);
    }

    Vector v5(v4, 0, 5);
    for (size_t i = 0; i < 5; ++i) {
        assert(v5[i] == static_cast<double>(i));
    }
    std::cout << "OK\n";
}

void run_all_tests() {
    std::cout << "Running Vector tests...\n";
    std::cout << "========================\n";

    test_basic_creation_and_access();
    test_copy_on_write();
    test_subvectors();
    test_assignment_operator();
    test_const_buffer();
    test_buffer_write();
    test_multiple_copies_and_cow();
    test_direct_acquire_methods();
    test_nested_subvectors();
    test_self_assignment();
    test_buffer_transfers();
    test_const_correctness();
    test_edge_cases();

    std::cout << "========================\n";
    std::cout << "All tests passed successfully!\n";
}

int main() {
    run_all_tests();
    return 0;
}
