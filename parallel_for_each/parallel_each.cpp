#include <future>
#include <algorithm>
#include <vector>
#include <functional>
#include <iostream>

using namespace std;

template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f)
{
    unsigned long const length = std::distance(first, last);

    if(!length)
        return;
    
    unsigned long const min_per_threads = 25;
    unsigned long const max_threads = 
                (length+min_per_threads - 1)/min_per_threads;
    
    unsigned long const harware_threads = 
            std::thread::hardware_concurrency();

    unsigned long const num_threads = 
            std::min(harware_threads!=0? harware_threads:2, max_threads);
    
    unsigned long const block_size = length/num_threads;

    std::vector<std::future<void>> futures(num_threads-1);
    std::vector<std::thread> threads(num_threads-1);
    
    Iterator block_start=first;
    for(unsigned long i = 0; i < (num_threads - 1); i++)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        std::packaged_task<void(void)> task(
            [=]()
            {
                cout << "task opeartion one" << endl;
                std::for_each(block_start, block_end,f);
            });
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task));
        block_start = block_end;
    }

    std::for_each(block_start, last, f);

    for(unsigned long i = 0; i < (num_threads-1); i++)
    {
        futures[i].get();
    }

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    return ;
}

void add_one(int &a)
{
    a++;
}

int main(int argc, char** argv)
{
    vector<int> sum_all(100,0);

    for(int i = 0; i < sum_all.size(); i++)
    {
        sum_all[i] = i+1;
    }

    std::function<void(int &)> func = add_one;

    parallel_for_each<std::vector<int>::iterator,std::function<void(int &)>>(sum_all.begin(), sum_all.end(), func);

    for(int i = 0; i < sum_all.size(); i++)
    {
        cout << sum_all[i] << ' ';
    }
    cout << endl;

    return 0;
}
