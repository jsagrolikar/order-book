#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "orderbook.h"
#include "utils.h"
#include <vector>
#include <thread>

class ThreadPool
{
private:
    std::vector<std::thread> workers;
    OrderQueue &queue;
    OrderBook &book;

public:
    ThreadPool(OrderBook &ob, OrderQueue &oq, size_t numThreads) : book(ob), queue(oq)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back(orderProcessor, std::ref(book), std::ref(queue));
        }
    }
    ~ThreadPool()
    {
        for (auto &worker : workers)
        {
            if (worker.joinable())
                worker.detach();
        }
    }
};

#endif // THREADPOOL_H
