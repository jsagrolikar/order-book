#include "../include/order.h"
#include "../include/orderbook.h"
#include "../include/threadpool.h"
#include "../include/utils.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <format>

int main()
{

    OrderBook orderBook;
    OrderQueue orderQueue;
    ThreadPool pool(orderBook, orderQueue, 4);

    uint64_t orderId = 1;

    // Generate some initial limit orders
    for (int i = 0; i < 50000; ++i)
    {
        pushRandomLimitOrder(orderQueue, orderId++);
    }

    // Generate a mix of limit and market orders
    for (int i = 0; i < 50000; ++i)
    {
        if (rand() % 2 == 0)
        {
            pushRandomLimitOrder(orderQueue, orderId++);
        }
        else
        {
            pushRandomMarketOrder(orderQueue, orderId++);
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
