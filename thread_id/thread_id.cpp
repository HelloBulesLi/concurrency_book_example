#include <thread>
#include <iostream>
#include <string>
#include <chrono>

using namespace std;

void func()
{
    cout << "this func:" << std::this_thread::get_id() << endl;
}
int main()
{
    std::thread t1(func);
    // t1.join();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread t2(func);
    // t2.join();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread t3(func);
    // t3.join();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t1.join();
    t2.join();
    t3.join();

    cout << "current concurrency:" << std::thread::hardware_concurrency() << endl;

    return 0;
}