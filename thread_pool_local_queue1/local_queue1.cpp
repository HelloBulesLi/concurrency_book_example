#include <deque>
#include <future>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <numeric>
#include <list>
#include <experimental/deque>

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
    bool try_pop_move(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value = std::move(data_queue.front());
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

class work_stealing_queue
{
private:
    typedef function_wrapper data_type;
    std::deque<data_type> the_queue;
    mutable std::mutex the_mutex;

public:
    work_stealing_queue()
    {}

    work_stealing_queue(const work_stealing_queue& other)=delete;
    work_stealing_queue& operator=(
        const work_stealing_queue& other)=delete;
    
    void push(data_type data)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.push_front(std::move(data));
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(data_type& res)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }

        res = std::move(the_queue.front());
        the_queue.pop_front();
        return true;
    }

    bool try_steal(data_type& res)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }

        res = std::move(the_queue.back());
        the_queue.pop_back();
        return true;
    }
};

class thread_pool
{
public:
    typedef function_wrapper task_type;
    // template<typename FunctionType>
    // std::future<typename std::result_of<FunctionType()>::type>
    // submit(FunctionType f)
    // {
    //     typedef typename std::result_of<FunctionType()>::type result_type;

    //     std::packaged_task<result_type()> task(std::move(f));
    //     std::future<result_type> res(task.get_future());
    //     work_queue.push_move(std::move(task));
    //     return res;
    // }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type>
    submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;

        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        if(local_work_queue)
        {
            cout << "current thread id is:" << this_thread::get_id() << endl;
            local_work_queue->push(std::move(task));
        }
        else
        {
            pool_work_queue.push_move(std::move(task));
        }
        return res;
    }

    // void run_pending_task()
    // {
    //     function_wrapper task;
    //     std::shared_ptr<function_wrapper> task_ptr = work_queue.try_pop_move();
    //     if(task_ptr.get())
    //     {
    //         task_ptr->call();
    //     }
    //     else
    //     {
    //         std::this_thread::yield();
    //     }
    // }

    void run_pending_task()
    {
        function_wrapper task;
        if(pop_task_from_local_queue(task) ||
            pop_task_from_pool_queue(task) ||
            pop_task_from_other_thread_queue(task))
        {
            task.call();
        }
        else
        {
            std::this_thread::yield();
        }
    }

    thread_pool():
        done(false)
    {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for(unsigned i = 0; i < thread_count;i++)
            {
                queues.push_back(std::unique_ptr<work_stealing_queue>(\
                                new work_stealing_queue));
                threads.push_back(
                        std::thread(&thread_pool::worker_thread, this, i));
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

    typedef std::queue<function_wrapper> local_queue_type;
    std::vector<std::unique_ptr<work_stealing_queue>> queues;

    static thread_local work_stealing_queue* local_work_queue;
    static thread_local unsigned self_index;

private:
    std::atomic_bool done;
    std::vector<std::thread> threads;
    threadsafe_queue<function_wrapper> pool_work_queue;

    void worker_thread(unsigned self_index_)
    {
        self_index = self_index_;
        local_work_queue = queues[self_index].get();

        while(!done)
        {
            run_pending_task();
        }
    }    

    bool pop_task_from_local_queue(task_type& task)
    {
        return local_work_queue && local_work_queue->try_pop(task);
    }

    bool pop_task_from_pool_queue(task_type& task)
    {
        return pool_work_queue.try_pop_move(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task)
    {
        for(unsigned i = 0; i < queues.size(); ++i)
        {
            unsigned const index = (self_index+i+1)%queues.size();
            if(queues[index].get() && queues[index]->try_steal(task))
            {
                return true;
            }
        }

        return false;
    }
};

thread_local work_stealing_queue* thread_pool::local_work_queue;
thread_local unsigned thread_pool::self_index;

template<typename T>
struct sorter
{
    thread_pool pool;

    std::list<T> do_sort(std::list<T>& chunk_data)
    {
        if(chunk_data.empty())
        {
            return chunk_data;
        }

        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());

        T const& partition_val = *result.begin();

        typename std::list<T>::iterator divide_point = 
            std::partition(
                chunk_data.begin(), chunk_data.end(),
                [&](T const& val){return val< partition_val;});
        
        std::list<T> new_lower_chunk;
        new_lower_chunk.splice(
            new_lower_chunk.end(),
            chunk_data,chunk_data.begin(),
            divide_point);
        
        std::future<list<T>> new_lower = 
            pool.submit(std::bind(
                    &sorter::do_sort,this,
                    std::move(new_lower_chunk)));

        std::list<T> new_higher(do_sort(chunk_data));

        result.splice(result.end(), new_higher);

        while(new_lower.wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready)
        {
            pool.run_pending_task();
        }

        result.splice(result.begin(), new_lower.get());

        return result;
    }
};

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }

    sorter<T> s;

    return s.do_sort(input);
}

int main(int argc, char** argv)
{
    std::list<int> test_list = {8,3,2,9,1,4,6,5,10,7};

    std::list<int> result_list = parallel_quick_sort<int>(test_list);

    for(auto p:result_list)
    {
        cout << p << ' ';
    }
    cout << endl;

    return 0;
}


