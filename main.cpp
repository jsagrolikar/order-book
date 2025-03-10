#include "book.h"
#include "utils.h"
#include <vector>
#include <thread>
#include <memory>
#include <iostream>

using namespace book;
using namespace utils;

int main()
{
    auto orderBook = std::make_unique<OrderBook>();
    auto orderQueue = std::make_unique<OrderQueue>();
    std::atomic<uint64_t> orderId{1};

    std::vector<std::thread> producers;
    for (int i = 0; i < 2; ++i)
    {
        producers.emplace_back(producer, std::ref(orderBook), std::ref(orderQueue), std::ref(orderId));
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < 2; ++i)
    {
        consumers.emplace_back(orderProcessor, std::ref(orderBook), std::ref(orderQueue));
    }

    for (auto &p : producers)
        p.join();
    for (auto &c : consumers)
        c.join();

    return 0;
}
