#include <iostream>
#include <string>
#include <thread>
#include <mutex>

using namespace std;

class Elem {
public:
    Elem(int a) {this->a = a;}
    friend void swap(Elem &one, Elem &two);
    friend void swap_data(Elem &one, Elem &two);
    void set_value(int a)
    {
        std::lock_guard<std::mutex> lock(m);
        this->a = a;
    }
private:
    int a;
    std::mutex m;
};

void swap(Elem &one, Elem &two)
{
    if(&one == &two)
    {
        return;
    }
    // std::lock(one.m, two.m);

    // std::lock_guard<std::mutex> lock_a(one.m, std::adopt_lock);
    // std::lock_guard<std::mutex> lock_b(two.m, std::adopt_lock);
    // std::scoped_lock<std::mutex, std::mutex> guard(one.m, two.m);
    std::unique_lock<std::mutex> lock_a(one.m, std::defer_lock);
    std::unique_lock<std::mutex> lock_b(two.m, std::defer_lock);
    std::lock(lock_a, lock_b);
    swap_data(one, two);
    cout << "swap value,one:" << one.a << ",two:" << two.a << endl;
}

void swap_data(Elem &one, Elem &two)
{
    int temp = one.a;
    one.a = two.a;
    two.a = temp;
}

void threadA(Elem &one)
{
    int count = 10;
    while(true)
    {
        count++;
        one.set_value(count);
        this_thread::sleep_for(chrono::seconds(2));
    }
}

void threadB(Elem &two)
{
    int count = 20;
    while(true)
    {
        count++;
        two.set_value(count);
        this_thread::sleep_for(chrono::seconds(2));

    }
}

void threadC(Elem &one, Elem & two)
{
    int count = 0;
    while(true)
    {
        count++;
        swap(one, two);
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main(int argc, char** argv)
{
    Elem one(10),two(100);
    std::thread t(threadA, std::ref(one));
    std::thread t1(threadB, std::ref(two));
    std::thread t3(threadC, std::ref(one), std::ref(two));

    while(true)
    {
        cout << "main thread alive" << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
    return 0;
}
