#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>
#include <functional>
#include <iostream>


using namespace std;

std::mutex m;
std::deque<std::packaged_task<void()>> tasks;


void task_process_thread()
{
    while(true)
    {
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(m);
            if(tasks.empty())
                continue;
            task = std::move(tasks.front());
            tasks.pop_front();
        }
        task();
    }
}

// template <typename Func, class... Args>
// std::future<void> post_task_for_gui_thread(Func&& f, Args&&... args)
// {
//     auto f1 = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
//     cout << "type name is: " << typeid(f1).name() << endl;
//     std::packaged_task<void()> task(f1);
//     std::future<void> res = task.get_future();
//     std::lock_guard<std::mutex> lk(m);
//     tasks.push_back(std::move(task));
//     return res;
// }

template <typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    std::packaged_task<void()> task(f);
    std::future<void> res = task.get_future();
    std::lock_guard<std::mutex> lk(m);
    tasks.push_back(std::move(task));
    return res;
}

// void print(int a)
// {
//     cout << "current elem is: " << a << endl;
// }

void print(int a)
{
    cout << "current elem is: " << a << endl;
}


void threadA()
{
    int n = 100;
    while(true)
    {
        std::function<void()> func = std::bind(print, n++);
        // std::function<void()> func = std::bind(print);
        // std::bind(std::forward<F>(func), std::forward<Args>(args)...)

        this_thread::sleep_for(chrono::seconds(2));
        post_task_for_gui_thread(func);
        // post_task_for_gui_thread(print, n);
    }
}

int main(int argc, char** argv)
{

    std::thread t1(threadA);

    std::thread t2(task_process_thread);

    t1.join();
    t2.join();

    return 0;
}
