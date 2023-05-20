#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <iostream>
#include <condition_variable>

using namespace std;

template<typename T>
class threadsafe_queue
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;
    std::condition_variable data_cond;

    node* get_tail()
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get() == get_tail())
        {
            return nullptr;
        }
        std::unique_ptr<node> old_head = std::move(head);
        head=std::move(old_head->next);
        return old_head;
    }

    std::unique_ptr<node> pop_head_nolock()
    {
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&]{return head.get() != get_tail();});
        return std::move(head_lock);
    }

    std::unique_ptr<node> wait_pop_head(T& value)
    {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value = std::move(*head->data);
        return pop_head_nolock();
    }

    std::unique_ptr<node> wait_pop_head()
    {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head_nolock();
    }

    std::unique_ptr<node> try_pop_head()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get() == get_tail())
        {
            return std::unique_ptr<node>();
        }
        return pop_head_nolock();
    }

    std::unique_ptr<node> try_pop_head(T& value)
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail())
        {
            return std::unique_ptr<node>();
        }
        value = std::move(*head->data);
        return pop_head_nolock();
    }

public:
    threadsafe_queue():
        head(new node), tail(head.get())
    {}

    threadsafe_queue(const threadsafe_queue& other)=delete;
    threadsafe_queue& operator=(const threadsafe_queue& other)=delete;

    // std::shared_ptr<T> try_pop()
    // {
    //     std::unique_ptr<node> old_head = pop_head();
    //     return old_head? old_head->data:std::shared_ptr<T>();
    // }

    void push(T new_value)
    {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        node* const new_tail = p.get();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(p);
        tail = new_tail;
        cout << "push a value" << endl;
        data_cond.notify_one();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_ptr<node> old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T& value)
    {
        std::unique_ptr<node> old_head = wait_pop_head(value);
    }

    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<node> const old_head=try_pop_head();
        return old_head? old_head->data:std::shared_ptr<T>();
    }
    bool try_pop(T& value)
    {
        std::unique_ptr<node> const old_head = try_pop_head(value);
        return old_head;

    }

    bool empty()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return (head.get() == get_tail());
    }
};

class DataType
{
public:
    DataType()
    {
        this->a = 0;
    }
    DataType(int a)
    {
        this->a = a;
    }
    DataType(DataType&& other)
    {
        this->a = other.a;
        other.a = 0;
    }
    int get_value()
    {
        return a;
    }
    DataType& operator=(const DataType &other)
    {
        this->a = other.a;
        return *this;
    }
private:
    int a;
};

void threadA(threadsafe_queue<DataType> &queue_test)
{
    int n = 0;
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(2));
        queue_test.push(DataType(n++));
    }
}
// void threadB(threadsafe_queue<DataType> &queue_test)
// {
//     while(true)
//     {
//         this_thread::sleep_for(chrono::seconds(2));
//         std::shared_ptr<DataType> temp =  queue_test.try_pop();
//         if(temp.get() != nullptr)
//         {
//             cout << "current top is: " << temp->get_value() << endl;
//         }
//     }
// }

void threadB(threadsafe_queue<DataType> &queue_test)
{
    while(true)
    {
        std::shared_ptr<DataType> temp =  queue_test.wait_and_pop();
        // if(temp.get() != nullptr)
        // {
            cout << "current top is: " << temp->get_value() << endl;
        // }
    }
}

int main(int argc, char** argv)
{
    threadsafe_queue<DataType> queue_test;

    std::thread t1(threadA, std::ref(queue_test));
    std::thread t2(threadB, std::ref(queue_test));

    t1.join();
    t2.join();
    return 0;
}
