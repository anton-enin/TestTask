#include <iostream>
#include <set>
#include <future>
#include <ranges>
#include <range/v3/all.hpp>
#include <algorithm>
#include "Implementation/Queue.h"

template<typename T>
void runner(size_t queueSize, std::multiset<T> srcMsg)
{
    MessageQueue<T> queue(queueSize);
    auto head = srcMsg.begin();

    std::vector<std::future<bool>> writer;
    std::vector<std::future<std::shared_ptr<T>>> reader;

    const auto read = [&reader, &queue](){reader.emplace_back(std::async(std::launch::async, [&queue]{return queue.pop();}));};

    const auto write = [&writer, &queue, &head]
    {
        const auto data = *head;
        ++head;
        writer.emplace_back(std::async(std::launch::async, [&queue, data]{return queue.push(data);}));
    };

    std::multiset<T> res;

    std::ranges::for_each(std::views::iota(0, 4), [&](int i) {
        int maxThreads = std::thread::hardware_concurrency();
        //it's user dependent part
        maxThreads = maxThreads != 0 ? maxThreads : 2;
        std::ranges::for_each(std::views::iota(0, maxThreads), [&, i](int j)
        {
            switch(i)
            {
            case 0:
                write();
                break;
            case 1:
                j<8 ? read() : write();
                break;
            case 2:
                j<6 ? write() : read();
                break;
            case 3:
                j<4 ? write() : read();
                break;
            }
        });

        for(auto &item : writer)
        {
            item.get();
        }
        writer.clear();

        for(auto &item : reader)
        {
            res.emplace(*(item.get()));
        }
        reader.clear();
    });

    std::ranges::for_each(res, [](auto item){std::cout<<item<<" ";});
    assert(res.size()==22);
}

int main()
{
    runner(10, std::views::iota(0, 26) | ranges::to<std::multiset<int>>);
}
