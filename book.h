#ifndef BOOK_H
#define BOOK_H

#include "order.h"
#include <queue>
#include <map>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <iostream>

class OrderQueue
{
private:
    std::queue<Order> queue;
    mutable std::mutex mutex;
    std::condition_variable cv;

public:
    void push(Order order)
    {
        {
            std::lock_guard lock(mutex);
            queue.push(std::move(order));
        }
        cv.notify_one();
    }

    std::optional<Order> pop()
    {
        std::unique_lock lock(mutex);
        cv.wait(lock, [this]
                { return !queue.empty(); });

        Order order = std::move(queue.front());
        queue.pop();
        return order;
    }
};

class OrderBook
{
private:
    std::map<double, std::deque<LimitOrder>> buyOrders;
    std::map<double, std::deque<LimitOrder>> sellOrders;
    mutable std::mutex mutex;

    /*
    Executes the order by matching it with the best available order in the book.
    This piece of the logic is largely common between LimitOrder and MarketOrder, so I templatize.
    */
    template <ValidOrderType T>
    void executeOrder(T &order, std::map<double, std::deque<LimitOrder>> &bookSide, bool timeDependent = false)
    {
        while (order.quantity > 0 && !bookSide.empty())
        {
            auto bestPriceIt = (order.side == Side::BUY) ? bookSide.begin() : std::prev(bookSide.end());
            auto &orderQueue = bestPriceIt->second;
            auto &limitOrder = orderQueue.front();

            uint64_t tradeQuantity = std::min(order.quantity, limitOrder.quantity);

            order.quantity -= tradeQuantity;
            limitOrder.quantity -= tradeQuantity;

            if (limitOrder.quantity == 0)
                orderQueue.pop_front();
            if (orderQueue.empty())
                bookSide.erase(bestPriceIt);

            if (timeDependent)
                break;
        }
    }

    /*
    Functions specifying action for the different types of orders.
    */
    void handleOrder(LimitOrder &&order)
    {
        addOrder(std::move(order));
        matchOrders();
    }

    void handleOrder(MarketOrder &&order)
    {
        fillOrder(std::move(order));
    }
    /*
     */

    /*
    Adds a limit order to the book.
    */
    void addOrder(LimitOrder const &order)
    {
        {
            auto &bookSide = (order.side == Side::BUY) ? buyOrders : sellOrders;
            bookSide[order.price].push_back(order);
        }
    }

    /*
    Fills a current market order using the book.
    */
    void fillOrder(MarketOrder &&marketOrder)
    {
        auto &bookSide = (marketOrder.side == Side::BUY) ? sellOrders : buyOrders;
        executeOrder(marketOrder, bookSide);
    }

    /*
    Matches limit orders already on the book that cross (buy price >= sell price).
    */
    void matchOrders()
    {
        while (!buyOrders.empty() && !sellOrders.empty())
        {
            auto bestBuy = std::prev(buyOrders.end());
            auto bestSell = sellOrders.begin();
            if (bestBuy->first < bestSell->first)
                break;

            auto buyOrder = bestBuy->second.front();
            auto sellOrder = bestSell->second.front();
            if (buyOrder.timestamp > sellOrder.timestamp)
            {
                executeOrder(buyOrder, sellOrders, true);
            }
            else
            {
                executeOrder(sellOrder, buyOrders, true);
            }
        }
    }

public:
    /*
    Small wrapper to be called by the order processor.
    */
    void processOrder(Order &&order)
    {
        std::lock_guard lock(mutex);
        std::visit([this](auto &&ord)
                   { handleOrder(std::move(ord)); }, std::move(order));
    }

    /*
    Prints an order book snapshot (top 5 bids and asks).
    Need to be a bit careful on the ask side since we need to get the lowest prices.
    */
    void printOrderBook()
    {
        std::lock_guard lock(mutex);

        std::string output = "\n=== ORDER BOOK SNAPSHOT ===\n";
        output += "Best Offers:\n";

        std::vector<std::pair<double, uint64_t>> topAsks;
        int count = 0;
        for (auto &[price, orders] : sellOrders)
        {
            if (count++ == 5)
                break;
            topAsks.emplace_back(price, orders.front().quantity);
        }

        std::reverse(topAsks.begin(), topAsks.end());

        for (const auto &[price, quantity] : topAsks)
        {
            output += std::format("Price: {:.2f} | Quantity: {}\n", price, quantity);
        }

        output += "Best Bids:\n";
        count = 0;
        for (auto it = buyOrders.rbegin(); it != buyOrders.rend(); ++it)
        {
            if (count++ == 5)
                break;
            output += std::format("Price: {:.2f} | Quantity: {}\n", it->first, it->second.front().quantity);
        }
        output += "===========================\n";

        std::cout << output;
    }
};

#endif // BOOK_H
