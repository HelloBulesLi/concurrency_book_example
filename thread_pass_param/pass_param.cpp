#include <thread>
#include <iostream>
#include <string>
#include <memory>

using namespace std;

class TESTA {
public:
    void print()
    {
        cout << "hello kitt" << endl;
    } 
};

void process_TESTA(std::unique_ptr<TESTA> ptr)
{
    ptr->print();
}

int main(int argc, char** argv)
{
    TESTA obj;
    std::thread t(&TESTA::print, &obj);
    t.join();

    std::unique_ptr<TESTA> obj_ptr = std::make_unique<TESTA>();

    std::thread t1(process_TESTA, std::move(obj_ptr));

    t1.join();
    return 0;
}
