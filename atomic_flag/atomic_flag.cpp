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

spinlock_mutex mut;
int n = 0;

int main(int argc, char** argv)
{

    std::thread t1([]{
        while(true)
        {
            mut.lock();
            cout << "current n is:" << n++ << endl;
            mut.unlock();
            this_thread::sleep_for(chrono::seconds(2));
        }
    });

    std::thread t2([]{
        while(true)
        {
            mut.lock();
            cout << "current n is:" << n++ << endl;
            mut.unlock();
            this_thread::sleep_for(chrono::seconds(2));
        }
    });

    std::thread t3([]{
        while(true)
        {
            mut.lock();
            cout << "current n is:" << n++ << endl;
            mut.unlock();
            this_thread::sleep_for(chrono::seconds(2));
        }
    });


    t1.join();
    t2.join();
    t3.join();
    return 0;
}
