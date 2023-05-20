#include <iostream>
#include <mutex>
#include <thread>
#include <boost/thread/barrier.hpp>

using namespace std;

boost::barrier b(5);

void worker(int id)
{
    cout << "worker " << id << " is wating barrier" << endl;
    b.count_down_and_wait();
    cout << id << " wait finish" << endl;
}

int main(int argc, char** argv)
{
    std::vector<std::thread> workers;
    for(int i = 0; i < 5; i++) {
        workers.emplace_back(worker, i);
    }

    for(auto& worker: workers)
    {
        worker.join();
    }

    return 0;
}
