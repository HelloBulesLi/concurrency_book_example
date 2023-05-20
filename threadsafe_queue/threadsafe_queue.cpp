#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <thread>
#include <iostream>

using namespace std;

template<typename T>
class threadsafe_queue
{
public:
    threadsafe_queue()
    {}
    threadsafe_queue(threadsafe_queue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
};

class Data {
public:
    Data()
    {
        a = 0;
    }
    Data(int a)
    {
        this->a = a;
    }
    int get_value()
    {
        return this->a;
    }
private:
    int a;
};

void threadA(threadsafe_queue<Data> &safe_queue)
{
    int n = 0;
    while(true)
    {
        std::this_thread::sleep_for(chrono::seconds(2));
        safe_queue.push(Data(n++));
    }
}

void threadB(threadsafe_queue<Data> &safe_queue)
{
    while(true)
    {
        Data head;
        safe_queue.wait_and_pop(head);
        cout << "current front is " << head.get_value() << endl;
    }
}

int main(int argc, char** argv)
{
    threadsafe_queue<Data> safe_queue;
    std::thread t1(threadA, std::ref(safe_queue));
    std::thread t2(threadB, std::ref(safe_queue));

    t1.join();
    t2.join();

    return 0;
}
