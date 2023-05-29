#include <future>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

template<typename Iterator, typename MatchType>
Iterator parallel_find_impl(Iterator first, Iterator last, MatchType match,
                            std::atomic<bool>& done)
{
    try
    {
        unsigned long const length = std::distance(first, last);
        unsigned long const min_per_thread = 25;

        if(length < (2*min_per_thread))
        {
            for(;(first != last)&& !done.load();++first)
            {
                if(*first == match)
                {
                    done.store(true);
                    return first;
                }
            }
            return last;
        }
        else 
        {
            Iterator const mid_point = first + (length/2);
            std::future<Iterator> async_result = 
                        std::async(&parallel_find_impl<Iterator, MatchType>,
                        mid_point, last, match, std::ref(done));
            Iterator const direct_result = 
                        parallel_find_impl(first, mid_point, match, done);
            return (direct_result == mid_point)?
                    async_result.get():direct_result;
        }
    }
    catch(...)
    {
        done.store(true);
        throw std::current_exception();
    }
    
}

template<typename Iterator,typename MatchType>
Iterator parallel_find(Iterator first,Iterator last,MatchType match)
{
    std::atomic<bool> done(false);
    return parallel_find_impl(first,last,match,done);
}



int main(int argc, char** argv)
{
    vector<int> sum_all(100,0);

    for(int i = 0; i < sum_all.size(); i++)
    {
        sum_all[i] = i+1;
    }

    vector<int>::iterator ret;
    try
    {
        ret =  parallel_find<vector<int>::iterator, int>(sum_all.begin(),sum_all.end(), 50);
    }
    catch(...)
    {
        cout << "encounter a exception" << endl;
    }

    if(ret != sum_all.end())
    {
        cout << "match element is " << *ret << endl;
    }
    else
    {
        cout << "can't find element" << endl;
    }

    return 0;
}