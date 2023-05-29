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
    
    unsigned long const min_per_thread = 25;

    if (length < (2*min_per_thread))
    {
        std::for_each(first, last, f);
    }
    else
    {
        Iterator const mid_point = first+length/2;
        std::future<void> first_half = 
            std::async(&parallel_for_each<Iterator, Func>, first, mid_point, f);
        cout << "async task create" << endl;
        parallel_for_each(mid_point, last, f);
        first_half.get();
    }
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
