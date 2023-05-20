#include <thread>
#include <iostream>
#include <string>
#include <mutex>

using namespace std;

std::shared_ptr<int> arr;

std::once_flag initial_flag;

void initial_func()
{
    arr = std::shared_ptr<int>(new int[100]);
    cout << "init end" << endl;
}

void foo()
{
    std::call_once(initial_flag, initial_func);
}
int main(int argc, char** argv)
{
    std::thread t1([]{
        while(true)
        {
            this_thread::sleep_for(chrono::seconds(2));
            foo();
        }
    });

    std::thread t2([]{
        while(true)
        {
            this_thread::sleep_for(chrono::seconds(2));
            foo();
        }
    });

    while(true)
    {
        this_thread::sleep_for(chrono::seconds(2));
        cout << "main thread alive" << endl;
    }
    return 0;
}
