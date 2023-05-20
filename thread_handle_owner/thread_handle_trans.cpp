#include <thread>
#include <iostream>
#include <string>
#include <memory>

using namespace std;

void some_function()
{
    while(1)
    {
        cout << "func" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void some_other_function()
{
    while(1)
    {
        cout << "other func" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
} 


int main(int argc, char** argv)
{
    std::thread t1(some_function);
    std::thread t2 = std::move(t1);
    cout << "remove t1 to t2" << endl;
    t1 = std::thread(some_other_function);
    std::thread t3 = std::move(t2);
    cout << "remove t3 to t1" << endl;
    t1 = std::move(t3);

    while(1)
    {
        cout << "main thread" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
