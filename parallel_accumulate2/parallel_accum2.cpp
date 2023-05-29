#include <vector>
#include <thread>
#include <algorithm>
#include <vector>
#include <iostream>
#include <numeric>
#include <future>
using namespace std;

// template<typename Iterator, typename T>
// struct accumulate_block
// {
//     void operator()(Iterator first, Iterator last, T& result)
//     {
//         cout << "result init is :" << result << endl;
//         result = std::accumulate(first, last, result);
//         cout << "result after acc is:" << result << endl;
//     }
// };


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

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);

    if(!length)
        return init;
    
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = 
            (length+min_per_thread - 1)/min_per_thread;

    unsigned long const hardware_threads = 
            std::thread::hardware_concurrency();
    cout << "hardware thread:" << hardware_threads << endl;
    
    unsigned long const num_threads = 
            std::min(hardware_threads!=0? hardware_threads:2, max_threads);
    
    cout << "num threads:" << num_threads << endl;
    
    unsigned long const block_size = length/num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::future<T>> futures(num_threads - 1);
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    for(unsigned long i = 0; i < (num_threads-1); i++)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        accumulate_block<Iterator, T> block;
        // std::packaged_task<T(Iterator, Iterator)> task(
        //     accumulate_block<Iterator, T>());
        std::packaged_task<T(Iterator, Iterator)> task(block);
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task), block_start, block_end);
        threads[i].detach();
        block_start = block_end;
    }
    T last_result = accumulate_block<Iterator, T>()(block_start, last);

    T result = init;
    for(unsigned long i = 0; i < (num_threads - 1); i++)
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
    return 0;
}