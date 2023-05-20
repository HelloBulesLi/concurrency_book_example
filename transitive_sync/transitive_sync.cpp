#include <atomic>
#include <thread>
#include <assert.h>
#include <iostream>

using namespace std;
std::atomic<int> data_test[5];
std::atomic<bool> sync1(false), sync2(false);

void thread_1()
{
    data_test[0].store(42, std::memory_order_relaxed);
    data_test[1].store(97, std::memory_order_relaxed);
    data_test[2].store(17, std::memory_order_relaxed);
    data_test[3].store(-141, std::memory_order_relaxed);
    data_test[4].store(2003, std::memory_order_relaxed);
    sync1.store(true, std::memory_order_release);
}

void thread_2()
{
    while(!sync1.load(std::memory_order_acquire));
    sync2.store(std::memory_order_release);
}

void thread_3()
{
    while(!sync2.load(std::memory_order_acquire));
    assert(data_test[0].load(std::memory_order_relaxed)==42);
    assert(data_test[1].load(std::memory_order_relaxed)==97);
    assert(data_test[2].load(std::memory_order_relaxed)==17);
    assert(data_test[3].load(std::memory_order_relaxed)==-141);
    assert(data_test[4].load(std::memory_order_relaxed)==2003);
    cout << "no assert happen" << endl;
}

int main(int argc, char** argv)
{
    std::thread t1(thread_1);
    std::thread t2(thread_2);
    std::thread t3(thread_3);
    t1.join();
    t2.join();
    t3.join();

    return 0;
}
