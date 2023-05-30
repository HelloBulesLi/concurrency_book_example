#include <deque>
#include <future>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

template<typename T>
class threadsafe_queue
{
public:
    threadsafe_queue()
    {}
    threadsafe_queue(threadsafe_queue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void push_move(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    std::shared_ptr<T> try_pop_move()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
};

class function_wrapper
{
private:
    struct impl_base {
        virtual void call()=0;
        virtual ~impl_base(){}
    };

    std::unique_ptr<impl_base> impl;

    template<typename F>
    struct impl_type: impl_base
    {
        F f;
        impl_type(F&& f_): f(std::move(f_)) {}
        void call() {f();}
    };
public:

    template<typename F>
    function_wrapper(F&& f):
        impl(new impl_type<F>(std::move(f)))
    {}

    function_wrapper()
    {}

    void call() {impl->call();}

    function_wrapper(function_wrapper&& other) noexcept:
        impl(std::move(other.impl))
    {}

    function_wrapper& operator=(function_wrapper&& other) noexcept
    {
        if(this != &other)
        {
            impl = std::move(other.impl);
        }
        return *this;
    }

    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&)=delete;
};

class thread_pool
{
public:
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type>
    submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;

        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        work_queue.push_move(std::move(task));
        return res;
    }

    thread_pool():
        done(false)
    {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for(unsigned i = 0; i < thread_count;i++)
            {
                threads.push_back(
                        std::thread(&thread_pool::worker_thread, this));
            }
        }
        catch(...)
        {
            done.store(true);
            throw;
        }
    }

    ~thread_pool()
    {
        done.store(true);
        for_each(threads.begin(), threads.end(), std::mem_fn(&thread::join));
    }

private:
    std::atomic_bool done;
    std::vector<std::thread> threads;
    threadsafe_queue<function_wrapper> work_queue;

    void worker_thread()
    {
        while(!done)
        {
            std::shared_ptr<function_wrapper> task_ptr = work_queue.try_pop_move();
            if(task_ptr.get())
            {
                task_ptr->call();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
};


class Foo
{
public:
    Foo(int a)
    {
        this->a = a;
    }
    int operator()()
    {
        cout << "cur value is:" << a << endl;
        return a;
    }
private:
    int a;
};

int main(int argc, char** argv)
{

    thread_pool pool;

    int n = 0;

    std::vector<future<int>> res;
    while(n < 20)
    {
        Foo test(n);
        res.push_back(pool.submit(test));
        this_thread::sleep_for(chrono::milliseconds(200));
        n++;
    }

    for(auto &p:res)
    {
        cout << p.get() << ' ';
    }
    cout << endl;

    return 0;
}
