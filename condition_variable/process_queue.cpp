#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <chrono>

using namespace std;

std::mutex mut;
std::queue<int> data_queue;
std::condition_variable data_cond;

void print(int i)
{
    cout << "current queue front is:" << i << endl; 
}
void data_preparation_thread()
{
    int n = 0;
    while(true)
    {
        std::this_thread::sleep_for(chrono::seconds(2));
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(n++);
        data_cond.notify_one();
    }
}

void data_process_thread()
{
    while(true)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, []{return !data_queue.empty();});
        int i = data_queue.front();
        data_queue.pop();
        lk.unlock();
        print(i);
    }
}

int main(int argc, char** argv)
{
    std::thread t1(data_preparation_thread);
    std::thread t2(data_process_thread);

    t1.join();
    t2.join();
    
    return 0;
}