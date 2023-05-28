#include <thread>
#include <vector>
#include <future>
#include <list>
#include <stack>
#include <iostream>
#include <memory>
#include <algorithm>

using namespace std;

struct empty_stack: std::exception
{
    const char* what() const throw()
    {
        return "empty stack";
    }
};

template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data=other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(
            std::make_shared<T>(std::move(data.top()))
        );
        data.pop();
        return res;
    }

    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value = std::move(data.top());
        data.top();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};


template<typename T >
struct sorter
{
    struct chunk_to_sort
    {
        std::list<T> data;
        std::promise<std::list<T>> promise;
    };

    threadsafe_stack<chunk_to_sort> chunks;
    std::vector<std::thread> threads;
    unsigned const max_thread_count;
    std::atomic<bool> end_of_data;

    sorter():
        max_thread_count(std::thread::hardware_concurrency() - 1),
        end_of_data(false)
    {}

    ~sorter()
    {
        end_of_data = true;
        for(unsigned i = 0; i < threads.size(); ++i)
        {
            threads[i].join();
        }
    }

    void try_sort_chunk()
    {
        std::shared_ptr<chunk_to_sort> chunk;
        try
        {
            chunk = chunks.pop();
        }
        catch(const struct empty_stack& e)
        {
            std::cerr << e.what() << '\n';
        }
        
        // std::shared_ptr<chunk_to_sort> chunk = chunks.pop();
        if(chunk)
        {
            sort_chunk(chunk);
        }
    }
    void sort_chunk(std::shared_ptr<chunk_to_sort> const& chunk)
    {
        chunk->promise.set_value(do_sort(chunk->data));
    }

    void sort_thread()
    {
        while(!end_of_data)
        {
            try_sort_chunk();
            std::this_thread::yield();
        }
    }

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
            std::partition(chunk_data.begin(), chunk_data.end(),
                            [&](T const& val){return val<partition_val;});
        chunk_to_sort new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(),
                                    chunk_data, chunk_data.begin(),
                                    divide_point);
        std::future<std::list<T>> new_lower = 
                                new_lower_chunk.promise.get_future();
        chunks.push(std::move(new_lower_chunk));

        if(threads.size() < max_thread_count)
        {
            threads.push_back(std::thread(&sorter<T>::sort_thread, this));
        }

        std::list<T> new_higher(do_sort(chunk_data));

        result.splice(result.end(), new_higher);

        while(new_lower.wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready)
        {
            try_sort_chunk();
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
    cout << "max thread count:" << s.max_thread_count << endl;
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