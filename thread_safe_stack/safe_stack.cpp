#include <exception>
#include <stack>
#include <mutex>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

struct empty_stack: std::exception
{
    const char* what() const throw()
    {
        return "empty stack";
    }
};

template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data=other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(
            std::make_shared<T>(std::move(data.top()))
        );
        data.pop();
        return res;
    }

    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value = std::move(data.top());
        data.top();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
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

void threadA(threadsafe_stack<DataType> &stack_test)
{
    int n = 0;
    while(true)
    {
        std::this_thread::sleep_for(chrono::seconds(2));
        stack_test.push(DataType(n++));
    }
}

void threadB(threadsafe_stack<DataType> &stack_test)
{
    while(true)
    {
        DataType temp;
        std::this_thread::sleep_for(chrono::seconds(1));
        try{
            stack_test.pop(temp);
        }
        catch(const struct empty_stack& e)  
        {
            cout << "exception: " << e.what() << endl;
        }
        cout << "current stack top is: " << temp.get_value() << endl;
    }
}

int main(int argc, char** argv)
{
    threadsafe_stack<DataType> stack_test;

    std::thread t1(threadA, std::ref(stack_test));
    std::thread t2(threadB, std::ref(stack_test));

    t1.join();
    t2.join();

    return 0;
}
