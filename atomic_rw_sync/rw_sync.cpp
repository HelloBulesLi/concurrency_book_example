#include <vector>
#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

int n = 100;
std::vector<int> data_test;
std::atomic_bool data_ready(false);

void read_thread()
{
    while(true)
    {
        if(data_ready.load(std::memory_order_acquire))
        {
            cout << "the last elem is " << data_test[0] << endl;
            data_ready.store(false, std::memory_order_release);
            
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void writer_thread()
{
    data_test.push_back(n++);
    data_ready.store(true, std::memory_order_release);
}


int main(int argc, char** argv)
{
    std::thread t1(read_thread);
    std::thread t2(writer_thread);

    t1.join();
    t2.join();

    return 0;
}
