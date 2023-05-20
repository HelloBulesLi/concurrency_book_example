#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include <iostream>
#include <chrono>

using namespace std;

template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue()
    {}

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
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
void threadB(threadsafe_queue<DataType> &queue_test)
{
    while(true)
    {
        DataType temp;
        this_thread::sleep_for(chrono::seconds(2));
        queue_test.wait_and_pop(temp);
        cout << "current top is: " << temp.get_value() << endl;
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
