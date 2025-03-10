#ifndef UTILS_H
#define UTILS_H

#include <atomic>
#include <random>
#include <thread>
#include <array>
#include "order.h"
#include "book.h"

namespace utils
{

    // Note: since this is a header file, choosing not to use namespace std::chrono_literals
    // to avoid polluting the global namespace.

    // how much time the threads should sleep each iteration, in ms.
    int constexpr SLEEP_TIME_MS = 10;
    // how often to print the order book, in number of orders processed.
    int constexpr ORDER_BOOK_DISPLAY_INTERVAL = 1000000;
    // how many orders to process in total.
    uint64_t constexpr MAX_ORDERS = 10000000;
    // counter to keep track of how many orders have been processed.
    std::atomic<uint64_t> orderCounter = 0;

    /*
    Pops (optional) orders from the queue and processes them with the book.
    */
    void orderProcessor(std::unique_ptr<book::OrderBook> &book, std::unique_ptr<book::OrderQueue> &queue)
    {
        while (orderCounter.load(std::memory_order_relaxed) < MAX_ORDERS)
        {
            if (auto orderOpt = queue->pop())
            {
                book->processOrder(std::move(*orderOpt)); // OrderBook handles it
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
    }

    /*
    Some helpful distributions for generating orders.
    */
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> BpriceDist(100.05, 0.02);
    std::normal_distribution<double> ApriceDist(99.95, 0.02);
    std::binomial_distribution<uint64_t> qtyDist(100, 0.5);
    std::uniform_real_distribution<double> unifDist(0.0, 1.0);
    std::uniform_int_distribution<int> bernoulliDist(0, 1);

    /*
    Generates orders and pushes them safely to the queue.
    */
    void producer(std::unique_ptr<book::OrderBook> &book, std::unique_ptr<book::OrderQueue> &queue, std::atomic<uint64_t> &orderId)
    {
        std::array<order::Side, 2> sides = {order::Side::BUY, order::Side::SELL};
        while (orderCounter.load(std::memory_order_relaxed) < MAX_ORDERS)
        {

            uint64_t id = orderId.fetch_add(1, std::memory_order_relaxed);

            if (unifDist(gen) < 0.8)
            {
                order::LimitOrder order{
                    id, sides[bernoulliDist(gen)],
                    (bernoulliDist(gen) == 0) ? BpriceDist(gen) : ApriceDist(gen), qtyDist(gen)};
                queue->push(std::move(order));
            }
            else
            {
                order::MarketOrder order{id, sides[bernoulliDist(gen)], qtyDist(gen)};
                queue->push(std::move(order));
            }

            orderCounter.fetch_add(1, std::memory_order_relaxed);
            int count = orderCounter.load(std::memory_order_relaxed);
            if (count % ORDER_BOOK_DISPLAY_INTERVAL == 0)
            {
                std::cout << std::format("\n{} orders produced.", count);
                book->printOrderBook();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
    }

} // namespace utils

#endif // UTILS_H