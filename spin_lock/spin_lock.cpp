#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

class spinlock_mutex
{
    std::atomic_flag flag;
public:
    spinlock_mutex():
        flag(ATOMIC_FLAG_INIT)
    {}
    void lock()
    {
        while(flag.test_and_set(std::memory_order_acquire));
    }
    void unlock()
    {
        flag.clear(std::memory_order_release);
    }
};

class spinlock_mutex spinlock;
static int n = 0;

void threadA()
{
    while(true)
    {
        spinlock.lock();
        n++;
        spinlock.unlock();
        this_thread::sleep_for(chrono::seconds(2));
    }
}

void threadB()
{
    while(true)
    {
        spinlock.lock();
        int x = n;
        spinlock.unlock();
        cout << "current n is " << x << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main(int argc, char** argv)
{
    std::thread t1(threadA);
    std::thread t2(threadB);

    t1.join();
    t2.join();

    return 0;
}

