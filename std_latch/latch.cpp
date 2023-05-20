// #include <experimental/latch>
// #include <experimental/latch>
#include <boost/thread/latch.hpp>

// #include <latch.hpp>
// #include <latch>
#include <thread>
#include <iostream>

using namespace std;
using namespace boost;

boost::latch my_latch(5);
// std::latch  my_latch(5);

void worker() {
    cout << "Working...\n";
    // my_latch.count_down();
    my_latch.count_down_and_wait();
    cout << "all wait finish" << endl;
}

int main(int argc, char** argv)
{
    std::vector<std::thread> threads;

    for( int i = 0; i < 5; i++) {
        threads.emplace_back(worker);
    }

    // my_latch.wait();
    // cout << "latch count down" << endl;

    for(auto &t: threads) {
        t.join();
    }

    cout << "All threads finished!\n";

    return 0;
}