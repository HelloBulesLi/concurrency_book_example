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

class flowcontrol_flag
{
private:
    std::mutex flag_mutex;
    std::condition_variable cv;
    bool flag = false;
public:
    flowcontrol_flag(){}

    void stop()
    {
        std::unique_lock<std::mutex> lk(flag_mutex);
        flag = true;
    }

    bool is_set()
    {
        std::unique_lock<std::mutex> lk(flag_mutex);
        return flag;
    }

    void resume()
    {
        std::unique_lock<std::mutex> lk(flag_mutex);
        flag = false;
        cv.notify_one();
    }

    void wait(std::atomic_bool* done_ptr)
    {
        std::unique_lock<std::mutex> lk(flag_mutex);
        cv.wait(lk, [this, done_ptr]{return !flag || done_ptr->load();});
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

    void stop_thread(int index)
    {
        pool_flow_control[index]->stop();
    }

    void resume_thread(int index)
    {
        pool_flow_control[index]->resume();
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
                pool_flow_control.emplace_back(new flowcontrol_flag());
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

    unsigned size()
    {
        return threads.size();
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

    std::vector<std::shared_ptr<flowcontrol_flag>> pool_flow_control;

    void worker_thread(unsigned self_index_)
    {
        self_index = self_index_;
        local_work_queue = queues[self_index].get();

        while(!done)
        {
            if(pool_flow_control[self_index]->is_set())
            {
                pool_flow_control[self_index]->wait(&done);
            }
            cout << "current run thread id is:" << this_thread::get_id() << endl;
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

template<typename Iterator, typename T>
T parallel_accumulate(thread_pool* pool_ptr,Iterator first, Iterator last, T init)
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
    // thread_pool pool;

    Iterator block_start=first;
    
    for(unsigned long i = 0; i < (num_blocks-1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        futures[i] = pool_ptr->submit(std::bind(accumulate_block<Iterator,T>(), block_start, block_end));
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
    vector<int> sum_all(1000,0);

    for(int i = 0; i < sum_all.size(); i++)
    {
        sum_all[i] = i+1;
    }

    thread_pool pool;

    cout << "sum size is : " << sum_all.size() << endl;
    unsigned long const length = std::distance(sum_all.begin(), sum_all.end());

    if(!length)
    {
        return 0;
    }

    unsigned long const block_size = 25;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();

    unsigned long const num_blocks = (length + block_size - 1)/block_size;

    unsigned long const num_threads = std::min((hardware_threads != 0)? hardware_threads:2,num_blocks);

    std::vector<std::future<int>> futures(num_blocks-1);
    // thread_pool pool;

    vector<int>::iterator block_start= sum_all.begin();
    
    for(unsigned long i = 0; i < (num_blocks-1); ++i)
    {
        vector<int>::iterator block_end = block_start;
        std::advance(block_end, block_size);
        futures[i] = pool.submit(std::bind(accumulate_block<vector<int>::iterator,int>(), block_start, block_end));
        block_start = block_end;
    }

    int last_result = accumulate_block<vector<int>::iterator, int>()(block_start, sum_all.end());
    int result = 0;

    for(unsigned i = 0; i < pool.size(); i++)
    {
        pool.stop_thread(i);
    }

    cout << "sleep start " << endl;
    this_thread::sleep_for(chrono::seconds(3));
    cout << "sleep end " << endl;

    for(unsigned i = 0; i < pool.size(); i++)
    {
        pool.resume_thread(i);
    }

    for(unsigned long i = 0; i < (num_blocks-1); ++i)
    {
        result += futures[i].get();
    }
    result += last_result;

    // return result;
    cout.flush();
    cout << "total sum is: " << result << endl;
    cout << "caculate sum is: " << (1+1000.0)*1000.0/2 << endl;
}


