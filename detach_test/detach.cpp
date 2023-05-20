#include <thread>
#include <iostream>
#include <memory>

using namespace std;

class TESTA {
public:
    TESTA(int a)
    {
        b = a;
    }
    ~TESTA() {
        cout << "release TESTA" << endl;
    }
private:
    int b;
};

int main(int argc, char** argv)
{
    std::shared_ptr<TESTA> ptr = std::make_shared<TESTA>(100);

    {
            int i = 800;
            std::thread t([i]{
                while(1)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    cout << "i is " << i << endl;
                }
            });
            t.detach();
    }

    int count = 0;
    int n = 3;
    while(count < n)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        count++;
    }
    return 0;
}
