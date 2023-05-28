#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>

using namespace std;

template<typename T>
class threadsafe_list
{
    struct node
    {
        std::mutex m;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;

        node():
            next()
        {}

        node(T const& value):
            data(std::make_shared<T>(value))
        {}
    };

    node head;

public:
    threadsafe_list()
    {}

    ~threadsafe_list()
    {
        remove_if([](T const&){return true;});
    }

    threadsafe_list(threadsafe_list const& other)=delete;
    threadsafe_list& operator=(threadsafe_list const& other)=delete;

    void push_front(T const& value)
    {
        std::unique_ptr<node> new_node(new node(value));
        std::lock_guard<std::mutex> lk(head.m);
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    template<typename Function>
    void for_each(Function f)
    {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            f(*next->data);
            current = next;
            lk = std::move(next_lk);
        }
    }

    template<typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p)
    {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            if(p(*next->data))
            {
                return next->data;
            }
            current = next;
            lk = std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }

    template<typename Predicate>
    void remove_if(Predicate p)
    {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->m);
            if(p(*next->data))
            {
                std::unique_ptr<node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lk.unlock();
            }
            else
            {
                lk.unlock();
                current = next;
                lk = std::move(next_lk);
            }
        }
    }
};

void threadA(threadsafe_list<int> &list)
{
    int n = 0;
    while(true)
    {
        list.push_front(n++);
        this_thread::sleep_for(chrono::seconds(2));
    }
}

void print(int a)
{
    cout << a << ' ';
}

bool search_local(int a)
{
    if(a%2 == 0)
    {
        return true;
    }

    return false;
}

void threadB(threadsafe_list<int> &list)
{
    function<void(int)> a = print;

    while(true)
    {
        list.for_each<function<void(int)>>(a);
        cout << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
}


void threadC(threadsafe_list<int> &list)
{
    int n = 0;
    function<bool(int)> a = search_local;

    while(true)
    {
        list.remove_if<function<bool(int)>>(a);
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main(int argc, char** argv)
{
    threadsafe_list<int> list;
    std::thread t(threadA, std::ref(list));
    std::thread t1(threadB, std::ref(list));
    std::thread t2(threadC, std::ref(list));

    t.join();
    t1.join();
    t2.join();

    return 0;
}