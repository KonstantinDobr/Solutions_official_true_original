#include <algorithm>
#include <cassert>
#include <iostream>

class List {
public:
    [[nodiscard]] bool empty() const { return head == nullptr; }

    void show() const {
        Node *cur = head;
        while (cur != nullptr) {
            std::cout << cur->val << ' ';
            cur = cur->next;
        }
    }

    void push_front(int val) {
        Node *new_head = new Node;
        new_head->val = val;
        new_head->next = head;
        if (head == nullptr) {
            tail = new_head;
        }
        head = new_head;
    }

    void push_back(int val) {
        Node *new_tail = new Node;
        new_tail->val = val;
        new_tail->next = nullptr;
        if (tail != nullptr) {
            tail->next = new_tail;
        } else {
            head = new_tail;
        }
        tail = new_tail;
    }

    void pop_front() {
        if (head == nullptr)
            return;
        Node *temp = head;
        if (tail == head) {
            delete head;
            head = nullptr;
            tail = nullptr;
            return;
        }
        head = head->next;
        delete temp;
    }

    void pop_back() {
        if (tail == nullptr)
            return;
        if (head == tail) {
            delete head;
            head = nullptr;
            tail = nullptr;
            return;
        }
        Node *cur = head;
        while (cur->next->next != nullptr) {
            cur = cur->next;
        }
        delete cur->next;
        cur->next = nullptr;
        tail = cur;
    }

    [[nodiscard]] int get() const {
        if (head == nullptr)
            return -1;
        Node *slow = head;
        Node *fast = head;
        while (true) {
            if (fast == nullptr)
                break;
            if (fast->next == nullptr)
                break;
            slow = slow->next;
            fast = fast->next->next;
        }
        return slow->val;
    }

    ~List() {
        while (!empty()) {
            pop_front();
        }
    }

private:
    struct Node {
        int val{};
        Node *next = nullptr;
    };

    Node *head = nullptr;
    Node *tail = nullptr;
};

int main() {
    {
        List list;
        assert(list.empty());
        list.show();
    }

    {
        List list;
        list.push_front(1);
        list.push_front(2);
        list.push_back(3);
        list.push_back(4);
        assert(!list.empty());
        assert(list.get() == 3);
        list.show();
    }

    {
        List list;
        list.push_front(1);
        list.push_front(2);
        list.push_front(3);
        list.pop_front();
        list.pop_back();
        assert(list.get() == 2);
        list.show();
    }

    {
        List list;
        list.pop_front();
        list.pop_back();
        list.push_front(42);
        list.pop_front();
        assert(list.empty());
    }

    {
        List list;
        list.push_back(1);
        assert(list.get() == 1);

        list.push_back(2);
        assert(list.get() == 2);

        list.push_back(3);
        assert(list.get() == 2);
    }

    {
        List list;
        for(int i = 1; i <= 5; ++i) {
            list.push_back(i);
        }
        assert(list.get() == 3);

        list.pop_front();
        list.pop_back();
        assert(list.get() == 3);
    }

    {
        List list;
        for(int i = 0; i < 1000; ++i) {
            list.push_back(i);
        }
    }

    std::cout << "OK\n";
    return 0;
}
