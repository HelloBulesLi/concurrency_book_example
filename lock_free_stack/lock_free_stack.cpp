#include <atomic>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
template<typename T>
class lock_free_stack
{
public:
    void push(T const& data)
    {
        node* const new_node = new node(data);
        new_node->next = head.load();
        while(!head.compare_exchange_weak(new_node->next, new_node));
    }

    std::shared_ptr<T> pop()
    {
        node* old_head = head.load();
        while(old_head &&
                !head.compare_exchange_weak(old_head, old_head->next));
        return old_head? old_head->data : std::shared_ptr<T>();
    }
private:
    struct node
    {
        std::shared_ptr<T> data;
        node* next = nullptr;
        node(T const& data_):
            data(std::make_shared<T>(data_))
        {}
    };
    std::atomic<node*> head = nullptr;
};

void threadA(lock_free_stack<int> &stack_test)
{
    int n = 0;
    while(true)
    {
        this_thread::sleep_for(chrono::seconds(2));
        stack_test.push(n++);
    }
}

void threadB(lock_free_stack<int> &stack_test)
{
    while(true)
    {
        this_thread::sleep_for(chrono::seconds(1));
        std::shared_ptr<int> ptr = stack_test.pop();
        if(ptr)
        {
            cout << "current pop is:" << *ptr << endl;
        }
    }
}

int main(int argc, char** argv)
{
    lock_free_stack<int> stack_test;
    std::thread t1(threadA, std::ref(stack_test));
    std::thread t2(threadB, std::ref(stack_test));

    t1.join();
    t2.join();
    return 0;
}
