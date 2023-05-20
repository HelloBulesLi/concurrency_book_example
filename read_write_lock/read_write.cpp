#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <iostream>

using namespace std;

class dns_entry {
public:
    dns_entry(int a = -1){
        this->a = a;
    }
    int a;
};

class dns_cache {
public:
    dns_entry find_entry(string const& domain) const
    {
        std::shared_lock<std::shared_mutex> lk(entry_mutex);
        std::map<std::string, dns_entry>::const_iterator it = entries.find(domain);
        return (it == entries.end())? dns_entry():it->second;
    }
    void update_or_add_entry(string const& domain, 
                            dns_entry const& dns_details)
    {
        std::lock_guard<std::shared_mutex> lk(entry_mutex);
        entries[domain] = dns_details;
    }
private:
    std::map<std::string, dns_entry> entries;
    mutable std::shared_mutex entry_mutex;
};

void threadA(dns_cache &cache)
{
    // class dns_entry entry(20);
    // class dns_entry entry1(30);
    // cache.update_or_add_entry("key1", entry);
    // cache.update_or_add_entry("key2", entry1);

    int count = 100;
    while(true)
    {
        this_thread::sleep_for(chrono::seconds(2));
        cache.update_or_add_entry("key1", dns_entry(20+count));
        cache.update_or_add_entry("key2", dns_entry(30+count));
        count++;
        cout << "thread A alive" << endl;
    }
}

void threadB(dns_cache &cache)
{
    while(true)
    {
        dns_entry one = cache.find_entry("key1");
        dns_entry two = cache.find_entry("key2");
        cout << "key1: " << one.a << ",key2:" << two.a << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
}

void threadC(dns_cache &cache)
{
    while(true)
    {
        dns_entry one = cache.find_entry("key1");
        dns_entry two = cache.find_entry("key2");
        cout << "threadC key1: " << one.a << ",key2:" << two.a << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main(int argc, char** argv)
{
    dns_cache cache;
    thread t(threadA, std::ref(cache));
    thread t1(threadB, std::ref(cache));
    thread t2(threadC, std::ref(cache));

    while(true)
    {
        this_thread::sleep_for(chrono::seconds(2));
        cout << "main thread alive" << endl;
    }    
    return 0;
}
