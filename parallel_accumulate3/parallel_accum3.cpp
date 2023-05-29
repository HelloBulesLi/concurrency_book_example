#include <vector>
#include <thread>
#include <algorithm>
#include <vector>
#include <iostream>
#include <numeric>
#include <future>
using namespace std;

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);
    unsigned long const max_chunk_size = 25;
    if(length <= max_chunk_size)
    {
        cout << "one acc operation" << endl;
        return std::accumulate(first, last, init);
    }
    else
    {
        Iterator mid_point = first;
        std::advance(mid_point, length/2);
        std::future<T> first_half_result = 
                std::async(parallel_accumulate<Iterator, T>,
                            first, mid_point,init);
        T second_half_result = parallel_accumulate(mid_point, last, T());
        return first_half_result.get()+second_half_result;
    }
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
