#include <deque>
#include <future>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <numeric>

using namespace std;

template<typename Iterator, typename T>
struct accumulate_block
{
    T operator()(Iterator first, Iterator last)
    {
        // cout << "result init is :" << result << endl;
        T result = std::accumulate(first, last, T());
        cout << "result after acc is:" << result << endl;
        return result;
    }
};

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

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);

    if(!length)
    {
        return init;
    }

    unsigned long const block_size = 25;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();

    unsigned long const num_blocks = (length + block_size - 1)/block_size;

    unsigned long const num_threads = std::min((hardware_threads != 0)? hardware_threads:2,num_blocks);
    
    std::vector<std::future<T>> futures(num_blocks-1);
    thread_pool pool;

    Iterator block_start=first;
    
    for(unsigned long i = 0; i < (num_blocks-1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        futures[i] = pool.submit(std::bind(accumulate_block<Iterator,T>(), block_start, block_end));
        block_start = block_end;
    }

    T last_result = accumulate_block<Iterator, T>()(block_start, last);
    T result = init;

    for(unsigned long i = 0; i < (num_blocks-1); ++i)
    {
        result += futures[i].get();
    }
    result += last_result;

    return result;

}

int main(int argc, char** argv)
{
    vector<int> sum_all(100,0);

    for(int i = 0; i < sum_all.size(); i++)
    {
        sum_all[i] = i+1;
    }

    cout << "sum size is : " << sum_all.size() << endl;
    int sum = parallel_accumulate<std::vector<int>::iterator, int>(sum_all.begin(),sum_all.end(),
                                                                        0);
    cout << "total sum is: " << sum << endl;
    cout << "caculate sum is: " << (1+100.0)*100.0/2 << endl;
}
