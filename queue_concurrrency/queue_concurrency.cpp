#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <iostream>
#include <thread>

using namespace std;

class empty_stack: std::exception
{
public:
    empty_stack(char* msg){this->msg = msg;}
    virtual const char* what() const throw() {
        // string str("empty stack");
        // cout << "empty stack" << endl;
        return msg;
    };
    ~empty_stack() throw() {};
private:
    char* msg;
};

template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    // indicate the mutex can be modified in const,
    // class member function
    mutable std::mutex m;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&)=delete;
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack("empty stack");
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack("empty stack");
        value=data.top();
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }

};

class Elem {
public:
    Elem(int a){this->a = a;}
    Elem(Elem &other){this->a = other.a;}
    Elem(Elem &&other) {
        this->a = other.a;
        other.a = 0;
    }
    int get_value()
    {return a;}
private:
    int a;
};

void threadA(threadsafe_stack<Elem> &stack){
    Elem a(1),b(2),c(3);
    stack.push(a);
    stack.push(b);
    stack.push(c);
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        cout << "thread A alive" << endl;
    }
};

void threadB(threadsafe_stack<Elem> &stack){
    Elem c(4),d(5),e(6);
    stack.push(c);
    stack.push(d);
    stack.push(e);
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        cout << "thread B alive" << endl;
    }
};

void threadC(threadsafe_stack<Elem> &stack){

    while(true)
    {
        try {
            std::shared_ptr<Elem> visit_ptr = stack.pop();
            cout << visit_ptr->get_value() << endl;
        }
        catch(const struct empty_stack &e)
        {
            // cout << "exeception: " << e.what() << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(20));
    };
};

int main(int argc, char** argv)
{
    threadsafe_stack<Elem> stack;

    std::thread t(threadA, std::ref(stack));
    std::thread t1(threadB, std::ref(stack));
    std::thread t2(threadC, std::ref(stack));

    while(true)
    {
        this_thread::sleep_for(chrono::seconds(2));
        cout << "main thread alive" << endl;
    }

    return 0;
}
